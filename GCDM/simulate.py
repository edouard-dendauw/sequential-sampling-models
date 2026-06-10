# -*- coding: utf-8 -*-
"""
Update: 10 June 2026
@author: edendauw

Note. Nothing to declare.


"""

import os
import numpy as np
import ctypes
import platform
import matplotlib.pyplot as plt

from scipy.stats import norm
from copy import deepcopy

# parameters ----------------------------
# os.chdir(FOLDER_PATH)
np.set_printoptions(precision=3, suppress=True)


# -------------------------------------------------------------------
# ------- LOAD MODEL ------------------------------------------------

match platform.system():
    case "Linux":   lib = ctypes.CDLL("./gcdm.so")
    case "Darwin":  lib = ctypes.CDLL("./gcdm.dylib")
    case "Windows": lib = ctypes.CDLL("./gcdm.dll")
    case _: raise ValueError

GCDM = lib.GCDM

"""
> void GCDM(float x_0, float v, float g, float r,
>     float xi, float lambda, float beta,
>     float Te, float Tr, float sv,
>     int *resp, float *RT, float *PMT, float *MT,
>     float *firstHit, float *firstHitLoc, int *coactiv,
>     float s, float dt, int n, int maxiter,
>     int rangeLow, int rangeHigh, double *randomTable) {...}
"""

# pre-set the types of the arguments
GCDM.argtypes = [
    ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float,
    ctypes.c_float, ctypes.c_float, ctypes.c_float,
    ctypes.c_float, ctypes.c_float, ctypes.c_float,
    ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_float),
    ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float),
    ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_int),
    ctypes.POINTER(ctypes.c_int),
    ctypes.c_float, ctypes.c_float, ctypes.c_int, ctypes.c_int,
    ctypes.c_int, ctypes.c_int, ctypes.POINTER(ctypes.c_double),
]
GCDM.restype = None


# -------------------------------------------------------------------
# ------- PREPARE SIMULATION ----------------------------------------

class Simulation:
    def __init__(self, x_0, v, g, r, xi, leak, beta, Te, Tr, sv, n_sim_trials):
        # model's main parameters
        self.x_0 = x_0
        self.v = v
        self.g = g
        self.r = r
        self.xi = xi
        self.leak = leak
        self.beta = beta
        self.Te = Te
        self.Tr = Tr

        # between-trial variability
        self.sv = sv

        # outputs - psycho chronometric
        self.resp = np.zeros(n_sim_trials, dtype=np.int32)
        self.RT   = np.zeros(n_sim_trials, dtype=np.float32)
        self.PMT  = np.zeros(n_sim_trials, dtype=np.float32)
        self.MT   = np.zeros(n_sim_trials, dtype=np.float32)
        # outputs - partial response related
        self.firstHit    = np.zeros(n_sim_trials, dtype=np.float32)
        self.firstHitLoc = np.zeros(n_sim_trials, dtype=np.int32)
        self.coactiv     = np.zeros(n_sim_trials, dtype=np.int32)

        # constants
        self.s = .1
        self.dt = .001
        self.maxiter = 15000
        self.n = n_sim_trials

        # LUT parameters (see Evans, 2019)
        # build LUT table for gaussian random number generator
        interval = .0001
        gran = np.arange(interval, 1, interval)
        use_table = norm.ppf(gran)
        self.randomTable = use_table
        self.rangeLow = 0
        self.rangeHigh = len(self.randomTable) - 1



# -------------------------------------------------------------------
# ------- __main__ RUNNING ZONE -------------------------------------
if __name__ == '__main__':

    args = {
        'x_0': 0, 'v': 0.5, 'g': 0.07, 'r': 0.04,
        'xi': 0, 'leak': 30, 'beta': 0.01,
        'Te': 0.170, 'Tr': 0.080, 'sv': 0,
        'n_sim_trials': 10000
    }

    # prepare the simulation
    sim = Simulation(**args)

    # run the simulations
    GCDM(
        sim.x_0, sim.v, sim.g, sim.r,
        sim.xi, sim.leak, sim.beta,
        sim.Te, sim.Tr, sim.sv,

        sim.resp.ctypes.data_as(ctypes.POINTER(ctypes.c_int)),
        sim.RT.ctypes.data_as(ctypes.POINTER(ctypes.c_float)),
        sim.PMT.ctypes.data_as(ctypes.POINTER(ctypes.c_float)),
        sim.MT.ctypes.data_as(ctypes.POINTER(ctypes.c_float)),

        sim.firstHit.ctypes.data_as(ctypes.POINTER(ctypes.c_float)),
        sim.firstHitLoc.ctypes.data_as(ctypes.POINTER(ctypes.c_int)),
        sim.coactiv.ctypes.data_as(ctypes.POINTER(ctypes.c_int)),

        sim.s, sim.dt, sim.n, sim.maxiter,
        sim.rangeLow, sim.rangeHigh,
        sim.randomTable.ctypes.data_as(ctypes.POINTER(ctypes.c_double)),
    )

    pred_Resp = deepcopy(sim.resp)
    pred_RT = deepcopy(sim.RT)
    pred_PMT = deepcopy(sim.PMT)
    pred_MT = deepcopy(sim.MT)

    # only keep trials that trajectory has converged
    mask = (pred_RT > 0)

    pred_RT_correct = pred_RT[mask][pred_Resp[mask] == 1]

    plt.hist(pred_RT_correct)
    plt.show()


# -------------------------------------------------------------------
"""
References: 

Evans, N. J. (2019). A method, framework, and tutorial for efficiently simulating models of decision-making. 
Behavior Research Methods, 51(5), 2390–2404. https://doi.org/10.3758/s13428-019-01219-z
"""
