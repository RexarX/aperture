// aperture.ext.cpu_root — CPU-copied roots (specs.md §4.21, api_design.md §8.1).
//
// CpuRoot<T> is T as the push-constant / push-data block. Pair with
// aperture::ext::cpu_root draws. Not core RootData<T> (a pointer in push
// constants). Mixing both in one entry point is rejected at CreateProgram.
//
// This file is the public include name. The type lands with the core Slang
// module; until then, treat this comment as the contract.
