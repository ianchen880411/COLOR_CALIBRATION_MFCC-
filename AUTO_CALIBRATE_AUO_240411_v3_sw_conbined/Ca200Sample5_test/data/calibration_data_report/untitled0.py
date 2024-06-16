# -*- coding: utf-8 -*-
"""
Created on Tue Jun 11 16:05:36 2024

@author: ian88
"""

import sys
import os
import colour
import numpy as np
import pandas as pd
import openpyxl
from colour.utilities import as_float
from math import log10 as log10
from math import log
from math import sqrt, exp
import math


np.set_printoptions(suppress=True, threshold=sys.maxsize)

def f(t):
    t_o = as_float(np.where(
            t > 0.008856,
            t ** (1.0 / 3.0),
            (903.3 * t + 16.0) / 116.0,
    ))
    return t_o

def CCT_to_xyz_nor(cct):
     xy = np.array(colour.CCT_to_xy(cct))
     xyz = np.append(xy, 1.0 - xy[0] - xy[1])
     
     xyz *= 1.0 / xy[1]
    
     return xyz
 
    
M_Bradford = np.array(
            [[0.8951000, 0.2664000, -0.1614000],
             [-0.7502000, 1.7135000, 0.0367000],
             [0.0389000, -0.0685000, 1.0296000]])

M_Bradford_inv = np.array(
            [[0.9869929, -0.1470543, 0.1599627],
             [0.4323053, 0.5183603, 0.0492912],
             [-0.0085287, 0.0400428, 0.9684867]])
 
def Chromatic_Adaptation(XYZ, XYZ_W_S, XYZ_W_D):
    XYZ_W_S = np.array(XYZ_W_S)
    XYZ_W_D = np.array(XYZ_W_D)

    XYZ_W_S /= max(XYZ_W_S)
    XYZ_W_D /= max(XYZ_W_D)    
    cone_res_d = M_Bradford.dot(XYZ_W_D)
    cone_res_s = M_Bradford.dot(XYZ_W_S)
    
    m =  np.array(
        [[cone_res_d[0]/cone_res_s[0], 0, 0],
         [0, cone_res_d[1]/cone_res_s[1], 0],
         [0, 0, cone_res_d[2]/cone_res_s[2]]])   
     
    bradford_m =(M_Bradford_inv.dot(m)).dot(M_Bradford)
    XYZ_D = bradford_m.dot(XYZ)

    return XYZ_D, bradford_m
 
def XYZ_to_Lab(XYZ, XYZ_r):
    XYZ_w_TARGET = CCT_to_xyz_nor(6504)

    XYZ, bradford_m = Chromatic_Adaptation(XYZ, XYZ_r, XYZ_w_TARGET)
    XYZ_r, bradford_m = Chromatic_Adaptation(XYZ_r, XYZ_r, XYZ_w_TARGET)
    
    xr = XYZ[0]/XYZ_r[0]
    yr = XYZ[1]/XYZ_r[1]
    zr = XYZ[2]/XYZ_r[2]
    
    fx = f(xr)
    fy = f(yr)
    fz = f(zr)
    
    L = 116.0 * fy - 16.0
    a = 500.0 * (fx - fy)
    b = 200.0 * (fy - fz)
    
    return np.array([L, a, b]) 
X = [122.4, 128.4, 140.9]
print(X[0]/sum(X)-0.3127, X[1]/sum(X)-0.3291)
XYZ_r = [595.0511, 624.3805, 881.8742]

XYZ_t = [523.4063, 549.2014, 775.666]

XYZ_m = [525.7223, 547.9941, 777.9651]

Lab_t = XYZ_to_Lab(XYZ_t, XYZ_r)

Lab_m = XYZ_to_Lab(XYZ_m, XYZ_r)
print(Lab_t, Lab_m)
print(colour.delta_E_CIE2000(Lab_t, Lab_m))


