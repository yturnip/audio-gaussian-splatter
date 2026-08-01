# Audio Gaussian Splatter (AGS)

Audio Gaussian Splatter is a JUCE-based C++ application that generates a 3D Gaussian manifold - a
procedurally distributed cloud of "audio splats" — and uses each splat's geometric attributes to
drive real-time DSP effects. It is a C++ implementation of the Spherical Fibonacci Gaussian
Mixture (SF-GM) framework described in the accompanying research, adapting 3D Gaussian
Splatting as a control structure for spectral processing rather than for visual rendering

## Concept

Each splat in the manifold carries a 10-column latent feature vector — position, density,
eccentricity, spectral-harmonic color, curvature, and surface normal — that maps geometric
properties to audio-processing parameters. Rather than treating spatialization as an
afterthought, the system treats each splat as a volumetric sound source whose orientation and
density intrinsically shape its spectral behavior.

## Architecture

The project is organized into three conceptual layers, matching the CMake target structure:

| Layer | CMake target | Responsibility |
|---|---|---|
| Manifold generation | `ags-manifold` | Generates the `GaussianManifold` and its `GaussianSplat` data, handles rotation. |
| DSP primitives | `spiral-dsp` | Real Spiral effect implementations (delay, filters, modulation, etc.), based on *Audio Effects: Theory, Implementation and Application*. |
| Parameter mapping & engine | `ags-params`, `ags-engine` | Binds GMM attributes to DSP parameters and runs the per-splat audio signal chain. |
| Application shell | `ags-app` | JUCE standalone/plugin host wiring the above together. |

### Data flow

1. **Generator** (`SphereBranchingGenerator` / `DomeBranchingGenerator`) produces a `GaussianManifold`
   containing many `GaussianSplat`s, each with position, density, eccentricity, shColor,
   curvature, and normal.
2. **`ManifoldRotator`** applies a rigid rotation to the entire manifold — both positions and
   normals — used for interactive rotation and, before any occlusion calculation, to keep each
   splat's facing direction accurate relative to the fixed listener convention.
3. **`GMMBinding`** declares which splat attribute (density, shColor, eccentricity, curvature,
   or none) drives a given parameter, with an optional invert flag.
4. **`ParameterMapper`** reads the bound attribute, normalizes it, and scales it into the target
   `EffectParameter`'s own range, respecting manual vs. GMM-driven modes.
5. **`EffectChain`** sequences one or more `EffectProcessor` wrappers (e.g. `SpiralDelayProcessor`),
   supporting add/remove/reorder/bypass per slot.
6. **`SplatAudioProcessor`** is the per-splat glue object: it owns one `EffectChain`, pulls a
   parameter snapshot from `ParameterMapper` each block, applies it to the chain, processes the
   incoming audio sample, and scales the result by `SplatOcclusion`'s normal-driven gain.
7. **`AudioEngine`** *(WIP)* will own many `SplatAudioProcessor`s and mix their outputs into the
   multichannel output bus.

## Building

```bash
git clone <repo-url>
cd audio-gaussian-splatter
cmake -B build -S .
cmake --build build
```

Requires CMake 3.22+, a C++17 compiler, and internet access for CMake's `FetchContent`-managed
JUCE and glm dependencies.

## Testing

Unit tests run through the `ags-manifold-tests` target using JUCE's `UnitTest` framework:

```bash
cmake --build build --target ags-manifold-tests
./build/tests/ags-manifold-tests
```

## Research background

This implementation is based on the *Spectral Manifold Splatting* research, which proposes
adapting 3D Gaussian Splatting as a high-dimensional control structure for spectral processing,
evaluated through a within-subjects listening study measuring Spectral Morphological Integrity,
Spatial Envelopment, and Texture Engulfment.

## References

Effect DSP implementations follow *Audio Effects: Theory, Implementation and Application*, 2nd
ed., by Reiss & McPherson.
