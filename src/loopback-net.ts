import { ErrNoLocation, type EmNet } from 'xash3d-fwgs';

type Ip = [number, number, number, number];

type Packet = {
  data: Uint8Array;
  ip: Ip;
  port: number;
};

type Sock = {
  id: number;
  family: number;
  type: number;
  protocol: number;
  ip: Ip;
  port: number;
  incoming: Packet[];
};

type Emish = {
  HEAPU8: Uint8Array;
  HEAP8: Int8Array;
  HEAP16: Int16Array;
  HEAP32: Int32Array;
  HEAPU32: Uint32Array;
  setValue: (ptr: number, value: number, type?: string) => void;
  writeArrayToMemory: (array: number[], buffer: number) => void;
  intArrayFromString: (s: string, dontAddNull?: boolean) => number[];
  writeSockaddr: (sa: number, family: number, addr: string, port: number, addrlen?: number) => number;
  readSockaddr: (sa: number, salen: number) => { family?: number; addr?: string; port?: number; errno?: number };
  AsciiToString: (ptr: number) => string | undefined;
  _malloc: (n: number) => number;
};

const EAGAIN = 73;
const AF_INET = 2;
const MAX_QUEUE = 256;

function ipText(ip: Ip): string {
  return `${ip[0]}.${ip[1]}.${ip[2]}.${ip[3]}`;
}

function isLocalHost(name: string): boolean {
  const n = name.trim().toLowerCase();
  return !n || n === 'localhost' || n === 'loopback' || n === '127.0.0.1' || n === '::1' || n === 'local';
}

/**
 * In-process UDP for the Xash WASM listen server.
 *
 * Engine imports `_emscripten_net_*` and return -1 without Module.net, so the
 * local client never completes CL_SendConnectPacket. Packets are routed by
 * destination port onto the peer socket (client 27005 ↔ server 27015).
 */
export class EfwLoopbackNet implements EmNet {
  em?: Emish;
  private lastId = 1000;
  private nextEphemeral = 27005;
  private readonly sockets = new Map<number, Sock>();
  sent = 0;
  received = 0;
  drops = 0;
  onLog?: (line: string) => void;

  init(em: unknown): void {
    if (this.em) return;
    this.em = em as Emish;
    this.onLog?.('net: loopback init');
  }

  private eagain(): number {
    const em = this.em;
    if (em) {
      try {
        em.setValue(ErrNoLocation(em as never), EAGAIN, 'i32');
      } catch {
        /* errno pointer unavailable */
      }
    }
    return -1;
  }

  private copyFromHeap(ptr: number, len: number): Uint8Array {
    const heap = this.em!.HEAPU8;
    const out = new Uint8Array(len);
    out.set(heap.subarray(ptr, ptr + len));
    return out;
  }

  private readDest(addrPtr: number): { ip: Ip; port: number } {
    const em = this.em!;
    const heap = em.HEAPU8;
    const ip: Ip = [heap[addrPtr + 4], heap[addrPtr + 5], heap[addrPtr + 6], heap[addrPtr + 7]];
    const port = (heap[addrPtr + 2] << 8) | heap[addrPtr + 3];
    return { ip, port };
  }

  private writeSrc(addrPtr: number, socklenPtr: number, ip: Ip, port: number): void {
    if (!addrPtr) return;
    const em = this.em!;
    const base16 = addrPtr >> 1;
    em.HEAP16[base16] = AF_INET;
    em.HEAP8[addrPtr + 2] = (port >> 8) & 0xff;
    em.HEAP8[addrPtr + 3] = port & 0xff;
    em.HEAP8[addrPtr + 4] = ip[0];
    em.HEAP8[addrPtr + 5] = ip[1];
    em.HEAP8[addrPtr + 6] = ip[2];
    em.HEAP8[addrPtr + 7] = ip[3];
    if (socklenPtr) em.HEAP32[socklenPtr >> 2] = 16;
  }

  private destSock(fromId: number, port: number): Sock | undefined {
    for (const sock of this.sockets.values()) {
      if (sock.id !== fromId && sock.port === port) return sock;
    }
    for (const sock of this.sockets.values()) {
      if (sock.id !== fromId) return sock;
    }
    return this.sockets.get(fromId);
  }

  private enqueue(to: Sock, packet: Packet): void {
    if (to.incoming.length >= MAX_QUEUE) {
      to.incoming.shift();
      this.drops++;
    }
    to.incoming.push(packet);
  }

  socket(family: number, type: number, protocol: number): number {
    const id = this.lastId++;
    this.sockets.set(id, {
      id,
      family,
      type,
      protocol,
      ip: [127, 0, 0, 1],
      port: 0,
      incoming: [],
    });
    this.onLog?.(`net: socket fd=${id} family=${family} type=${type}`);
    return id;
  }

  bind(fd: number, sockaddrPtr: number, _socklenPtr?: number): number {
    const sock = this.sockets.get(fd);
    if (!sock || !this.em) return -1;
    const dest = this.readDest(sockaddrPtr);
    sock.ip = dest.ip[0] || dest.ip[3] ? dest.ip : [127, 0, 0, 1];
    sock.port = dest.port || this.nextEphemeral++;
    this.onLog?.(`net: bind fd=${fd} ${ipText(sock.ip)}:${sock.port}`);
    return 0;
  }

  sendto(fd: number, bufPtr: number, bufLen: number, _flags: number, sockaddrPtr: number, _socklenPtr: number): number {
    const from = this.sockets.get(fd);
    if (!from || !this.em) return -1;
    const dest = this.readDest(sockaddrPtr);
    const to = this.destSock(fd, dest.port);
    if (!to) return bufLen;
    this.enqueue(to, {
      data: this.copyFromHeap(bufPtr, bufLen),
      ip: from.ip,
      port: from.port || 27005,
    });
    this.sent++;
    if (this.sent <= 8 || this.sent % 50 === 0) {
      this.onLog?.(
        `net: sendto #${this.sent} ${bufLen}b fd=${fd}→${to.id} ${ipText(dest.ip)}:${dest.port} q=${to.incoming.length}`,
      );
    }
    return bufLen;
  }

  sendtoBatch(
    fd: number,
    bufsPtr: number,
    lensPtr: number,
    count: number,
    flags: number,
    sockaddrPtr: number,
    socklenPtr: number,
  ): number {
    const em = this.em;
    if (!em) return -1;
    let total = 0;
    for (let i = 0; i < count; i++) {
      const size = em.HEAP32[(lensPtr >> 2) + i];
      const ptr = em.HEAP32[(bufsPtr >> 2) + i];
      const n = this.sendto(fd, ptr, size, flags, sockaddrPtr, socklenPtr);
      if (n > 0) total += n;
    }
    return total;
  }

  /** WASM glue looks up snake_case sendto_batch. */
  sendto_batch(
    fd: number,
    bufsPtr: number,
    lensPtr: number,
    count: number,
    flags: number,
    sockaddrPtr: number,
    socklenPtr: number,
  ): number {
    return this.sendtoBatch(fd, bufsPtr, lensPtr, count, flags, sockaddrPtr, socklenPtr);
  }

  recvfrom(fd: number, bufPtr: number, bufLen: number, _flags: number, sockaddrPtr: number, socklenPtr: number): number {
    const sock = this.sockets.get(fd);
    if (!sock || !this.em) return this.eagain();
    const packet = sock.incoming.shift();
    if (!packet) return this.eagain();
    const n = Math.min(bufLen, packet.data.length);
    if (n > 0) this.em.HEAPU8.set(packet.data.subarray(0, n), bufPtr);
    this.writeSrc(sockaddrPtr, socklenPtr, packet.ip, packet.port);
    this.received++;
    if (this.received <= 8 || this.received % 50 === 0) {
      this.onLog?.(`net: recvfrom #${this.received} ${n}b fd=${fd} from ${ipText(packet.ip)}:${packet.port}`);
    }
    return n;
  }

  send(fd: number, bufPtr: number, bufLen: number, flags: number): number {
    const sock = this.sockets.get(fd);
    if (!sock) return -1;
    const em = this.em;
    if (!em) return -1;
    const sa = em._malloc(16);
    em.writeSockaddr(sa, AF_INET, ipText(sock.ip), sock.port || 27015);
    const n = this.sendto(fd, bufPtr, bufLen, flags, sa, 0);
    return n;
  }

  recv(fd: number, bufPtr: number, bufLen: number, flags: number): number {
    return this.recvfrom(fd, bufPtr, bufLen, flags, 0, 0);
  }

  connect(_fd: number, _sockaddrPtr: number, _socklenPtr: number): number {
    return 0;
  }

  select(n: number, rfds: number, _wfds: number, _efds: number, _tv: number): number {
    if (!this.em) return 0;
    let ready = 0;
    for (const sock of this.sockets.values()) {
      if (sock.incoming.length === 0) continue;
      ready++;
      if (!rfds || sock.id >= n) continue;
      const bit = 1 << (sock.id % 8);
      this.em.HEAPU8[rfds + (sock.id >> 3)] |= bit;
    }
    return ready;
  }

  gethostname(namePtr: number, _namelenPtr?: number): number {
    if (!this.em) return -1;
    const name = 'localhost';
    this.em.writeArrayToMemory(this.em.intArrayFromString(name, true), namePtr);
    return 0;
  }

  gethostbyname(_hostnamePtr: number): number {
    return 0;
  }

  getsockname(fd: number, sockaddrPtr: number, socklenPtr: number): number {
    const sock = this.sockets.get(fd);
    if (!sock || !this.em) return -1;
    this.em.writeSockaddr(sockaddrPtr, sock.family || AF_INET, ipText(sock.ip), sock.port, socklenPtr);
    return 0;
  }

  closesocket(fd: number): number {
    return this.sockets.delete(fd) ? 0 : -1;
  }

  getaddrinfo(hostnamePtr: number, restrictPtr: number, _hintsPtr: number, addrinfoPtr: number): number {
    const em = this.em;
    if (!em) return -1;
    const host = em.AsciiToString(hostnamePtr) ?? 'localhost';
    const service = restrictPtr ? em.AsciiToString(restrictPtr) ?? '' : '';
    const port = /^\d+$/.test(service) ? Number(service) : 0;
    const ip = isLocalHost(host) ? '127.0.0.1' : host.includes('.') ? host : '127.0.0.1';
    const sa = em._malloc(16);
    em.writeSockaddr(sa, AF_INET, ip, port);
    const ai = em._malloc(32);
    em.HEAP32[(ai + 4) >> 2] = AF_INET;
    em.HEAP32[(ai + 8) >> 2] = 2;
    em.HEAP32[(ai + 12) >> 2] = 17;
    em.HEAPU32[(ai + 24) >> 2] = 0;
    em.HEAPU32[(ai + 20) >> 2] = sa;
    em.HEAP32[(ai + 16) >> 2] = 16;
    em.HEAP32[(ai + 28) >> 2] = 0;
    em.HEAPU32[addrinfoPtr >> 2] = ai;
    this.onLog?.(`net: getaddrinfo ${host} -> ${ip}:${port}`);
    return 0;
  }

  summary(): string {
    return `sent=${this.sent} recv=${this.received} drops=${this.drops} socks=${this.sockets.size}`;
  }
}
