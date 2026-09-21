import { unzipSync } from 'fflate';

/* Inflate valve.zip off the UI thread. pak0.pak is a 52MB deflate and
   unzipSync on the main thread left the launch button disabled for minutes. */

type Ok = { names: string[]; buffers: ArrayBuffer[] };
type Fail = { error: string };

self.onmessage = (ev: MessageEvent<ArrayBuffer>) => {
  const post = self as unknown as {
    postMessage: (data: Ok | Fail, transfer?: Transferable[]) => void;
  };
  try {
    const zip = new Uint8Array(ev.data);
    const entries = unzipSync(zip);
    const names: string[] = [];
    const buffers: ArrayBuffer[] = [];
    for (const [name, data] of Object.entries(entries)) {
      names.push(name);
      const copy = data.slice();
      buffers.push(copy.buffer);
    }
    post.postMessage({ names, buffers }, buffers);
  } catch (err) {
    post.postMessage({ error: err instanceof Error ? err.message : String(err) });
  }
};
