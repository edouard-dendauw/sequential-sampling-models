# Gated Cascade Diffusion Model (GCDM)

## Reference

Dendauw, E., Evans, N. J., Logan, G. D., Haffen, E., Bennabi, D., Gajdos, T., & Servant, M. (2024). 
The gated cascade diffusion model: An integrated theory of decision making, motor preparation, and motor execution. 
*Psychological Review*, 131(4), 825–857. https://doi.org/10.1037/rev0000464

**OSF**: https://osf.io/4unw6 

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
- The response is issued when inputs to muscle fibers (neural drive) reach a threshold level of
  activation (response threshold).

**Definitions :**

| Name                                 | Label     | Equation                                                                            |
|--------------------------------------|-----------|-------------------------------------------------------------------------------------|
| decision variable                    | `x`       | $dx(t) = vdt + \sigma dW(t), \quad x(0) = x_0$                                      |
| corrupted decision variable          | `tilde_x` | $\tilde{x}(t) = x(t) + \xi U(t), \quad \tilde{x}(0) = x_0$                          |
| motor preparation variable           | `y`       | $d\tilde{y}(t) = \lambda(\tilde{x}(t) - \tilde{y}(t)) dt, \quad \tilde{y}(0) = x_0$ |
| neural drives for the right effector | `z_R`     | $z_R(t) = \max(0,  \tilde{y}(t) - g)$                                               |
| neural drives for the left effector  | `z_L`     | $z_L(t) = \max(0,  -\tilde{y}(t) - g)$                                              |

*Notes:*
- $\tilde{y}(t)$ is implemented as `y` in the code for readability, but corresponds to Equation 8 in Dendauw et al. (2024), 
  as it depends on the corrupted decision variable.

## Contents

- GCDM.c : core simulation code
- GCDM.py : Python equivalent
- simulation.py : Python wrapper (ctypes)

## Usage

The model can be called from Python via `ctypes`. See `simulation.py` for an example.

## Notes

- Non-response trials are identified by `RT == -1`.
- Full model equations, parameters, and implementation details are documented in `GCDM.c`.

## Version

`v2024-paper` corresponds to the version used in Dendauw et al. (2024).

