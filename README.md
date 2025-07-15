# dmald

Simple daemon to set value of CPU DMA Latency (`/dev/cpu_dma_latency`)

## Usage

```bash
dmald <latency_value> #(or -1 to terminate daemon)
```

- The first process will be the daemon, and others will send signal to the daemon to change the value.  
- You may need to be **root** or a member of **realtime**.

## Why

Some DAW like Ardour built-in the function of changing latency of CPU DMA, but hard to find a simple tool to set the latency when I want to run some other standalone application (e.g. Carla) and want to avoid too many XRUNs.
