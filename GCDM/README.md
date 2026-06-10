# Gated Cascade Diffusion Model (GCDM)

## Reference

Dendauw, E., Logan, G. D., Schall, J. D., Gajdos Preuss, T., Servant, M. (in press).
Separating decision and motor contributions to behavioral biases induced by manipulating stimulus probability.
*Cognitive Psychology*. https://doi.org/10.1016/j.cogpsych.2026.101818 (OSF Project: https://osf.io/8z3nj)

Dendauw, E., Evans, N. J., Michel, E., Garnier-Allain, A., Gajdos Preuss, T., & Servant, M. (2025).  
Deciding with muscles. 
*PsyArXiv preprint*. https://doi.org/10.31234/osf.io/7uwgn_v2  

Dendauw, E., Evans, N. J., Logan, G. D., Haffen, E., Bennabi, D., Gajdos, T., & Servant, M. (2024). 
The gated cascade diffusion model: An integrated theory of decision making, motor preparation, and motor execution. 
*Psychological Review*, 131(4), 825–857. https://doi.org/10.1037/rev0000464 (OSF Project: https://osf.io/4unw6)

## Overview

- A decision variable accumulates noisy sensory evidence.
- It is continuously transmitted to motor areas of the brain that prepare the response
  (e.g., premotor and primary motor cortices for hand movements).
- Because the brain is a noisy information processing system,
  the decision variable is likely corrupted by noise during transmission.
- In this context, the motor preparation process is modeled as a Kalman-Bucy filter
  to optimally recover the original decision variable from noise.
- The transmission of information from motor preparation to muscle fibers is also continuous,
  but regulated by a gate (implemented as a constant inhibition).
  The gate determines the minimum amount of motor preparation required to excite muscle fibers
  and serves two main purposes: shield the system against unwanted behaviors and prevent low
  levels of accumulated evidence from exciting muscle fibers.
- Additionally, an evidence-independent urgency signal at the motor preparation level is added
  to the motor prepration to drive muscle activation to the response threshold when the quality 
- of evidence is low or time pressure is high (see Discussion of Dendauw et al. 2024).
- The response is issued when inputs to muscle fibers (neural drive) reach a threshold level of
  activation (response threshold).

**Definitions :**

| Name                                 | Label | Equation                                                    |
|--------------------------------------|-------|-------------------------------------------------------------|
| decision variable                    | `x`   | $dx(t) = vdt + \sigma dW(t), \quad x(0) = x_0$              |
| corrupted decision variable          | `cx`  | $\tilde{x}(t) = x + \xi U(t), \quad \tilde{x}(0) = x_0$     |
| motor preparation variable           | `y`   | $dy(t) = \lambda(\tilde{x}(t) - y(t)) dt, \quad y(0) = x_0$ |
| evidence-independent urgency signal  | `u`   | $u(t) = \beta t$                                            |
| neural drives for the right effector | `z_R` | $z_R(t) = \max(0,  y(t) - g + u(t))$                        |
| neural drives for the left effector  | `z_L` | $z_L(t) = \max(0,  -y(t) - g + u(t))$                       |


## Contents

- `gcdm.c`      : core simulation code  
- `gcdm.so`     : compiled shared library for Linux systems  
- `gcdm.dylib`  : compiled shared library for macOS (Apple Silicon / Intel)  
- `gcdm.dll`    : compiled shared library for Windows
- `gcdm.py`     : Python implementation of the model  
- `simulate.py` : Python wrapper using `ctypes` to call the compiled library  

## Usage

The model can be called from Python via `ctypes`. See `simulation.py` for an example.

## Notes

- Non-response trials are identified by `RT == -1`.
- Full model equations, parameters, and implementation details are documented in `GCDM.c`.

## Version

`v2026.1` is the updated version used for ongoing analyses.
> **Warning**  
> This implementation corresponds to an ongoing version of the model described in the preprint above.  
> For the peer-reviewed version, see `v2024-paper`.

