# -*- coding: utf-8 -*-
"""
Created on Tue Dec 13 08:39:22 2022

@author: ian88
"""

import os
import numpy as np
import pandas as pd

'AUO_native'
XYZR = np.array([401.187317,178.678558,3.199178]) #RGB 1023, 0, 0
XYZG = np.array([151.480087,519.197937,51.873894]) #RGB 0,1023, 0
XYZB = np.array([166.068314, 54.240467, 891.829224]) #RGB 0, 0, 1023
XYZW = np.array([718.80365, 750.381531, 947.157104]) #RGB 1023, 1023, 1023
XYZ0 = np.array([0.505129, 0.541984, 1.009509]) #RGB 0, 0, 0

'''
'INN_native'
XYZR = np.array([302.417816, 138.637573, 4.292623]) #RGB 1023, 0, 0
XYZG = np.array([179.312927, 443.643799, 47.899467]) #RGB 0,1023, 0
XYZB = np.array([106.868942, 39.873371, 606.846863]) #RGB 0, 0, 1023
XYZW = np.array([594.82959, 625.800476, 678.746826]) #RGB 1023, 1023, 1023
XYZ0 = np.array([0.1892, 0.207, 0.3545]) #RGB 0, 0, 0
'''
def RGB_to_XYZ(Matrix, RGB, trc, Y_max):
    rgb = np.array(RGB)/1023
    v = [rgb[0]**trc, 
         rgb[1]**trc,
         rgb[2]**trc] 
    
    XYZ = Matrix.dot(v)*Y_max
    return XYZ 

def Build_Matrix_RGB_to_XYZ(XYZ_R, XYZ_G, XYZ_B, XYZ_0, Y_max):
    XYZR_nor = (XYZ_R-XYZ_0)/Y_max
    XYZG_nor = (XYZ_G-XYZ_0)/Y_max
    XYZB_nor = (XYZ_B-XYZ_0)/Y_max
    
    XYZR_nor_sum = sum(XYZR_nor)
    XYZG_nor_sum = sum(XYZG_nor)
    XYZB_nor_sum = sum(XYZB_nor)

    xR = XYZR_nor[0]/XYZR_nor_sum
    xG = XYZG_nor[0]/XYZG_nor_sum
    xB = XYZB_nor[0]/XYZB_nor_sum
    
    yR = XYZR_nor[1]/XYZR_nor_sum
    yG = XYZG_nor[1]/XYZG_nor_sum
    yB = XYZB_nor[1]/XYZB_nor_sum
            
    zR = XYZR_nor[2]/XYZR_nor_sum
    zG = XYZG_nor[2]/XYZG_nor_sum
    zB = XYZB_nor[2]/XYZB_nor_sum

    Xi = np.array(
        [[xR, xG, xB],
         [yR, yG, yB],
         [zR, zG, zB]])        
        
    Sm = np.array(
        [[XYZR_nor_sum, 0, 0],
         [0, XYZG_nor_sum, 0],
         [0, 0, XYZB_nor_sum]])
    
    M = Xi.dot(Sm)
    return M

M = Build_Matrix_RGB_to_XYZ(XYZR, XYZG, XYZB, XYZ0, XYZW[1])

gamut_target_str = input("Target Gamut? ")
cct_target_str, gamma_target_str = map(str, input("Target CCT、Gamma?  input with space(ex 6500 2.2): ").split())

base_path = current_path = os.path.dirname(os.path.abspath(__file__))

if gamma_target_str == '0':
    gamma_lut_path = base_path + "/lut/trc/dicom/lut_" + gamut_target_str + "_" + cct_target_str + "_dicom.csv"

else:
    gamma_target_str = gamma_target_str + '0'
    gamma_lut_path = base_path + "/lut/trc/gamma/lut_" + gamma_target_str + ".csv"

matrix_path = base_path + '/matrix/' +  gamut_target_str + '/' + "matrix_" + gamut_target_str + "_" +cct_target_str + ".csv"
one_d_lut_path = base_path + "/lut/1d_lut.csv"

lut = []
for i in range(0, 1024):
    lut.append([i,i,i])
    
try:  
    df = pd.read_csv(gamma_lut_path)
    sourcedata = df.values
    gamma_lut = np.vstack([sourcedata[:, 1:4]])
    print('\ngamma_lut csv: ', gamma_lut_path)
except:
    print('Invalid gamma_lut csv!', gamma_lut_path)
    os.system("pause")
    
try:  
    df = pd.read_csv(matrix_path, header = None)
    sourcedata = df.values
    matrix = np.vstack([sourcedata[:, 0:3]])
    print('matirx csv: ', matrix_path)
except:
    print('Invalid matirx csv!', matrix_path)
    os.system("pause")
    
try:  
    df = pd.read_csv(one_d_lut_path)
    sourcedata = df.values
    one_d_lut = np.vstack([sourcedata[:, 1:4]])
    print('one_d_lut csv: ', one_d_lut_path)
except:
    print('Invalid one_d_lut csv!', one_d_lut_path)
    os.system("pause")

while(True):
    try:
        R, G, B = map(int, input("RGB?  input with space(ex 1023 1023 1023): ").split())
    except:
        break
    
    #gamma
    print("\nRaw RGB: ", R, G, B)
    R = gamma_lut[R][0]
    G = gamma_lut[G][1]
    B = gamma_lut[B][2]
    print("gamma RGB: ", R, G, B)
    
    
    #gamut cct matrix
    RGB = np.array([R,G,B])/1023.0
    RGB = matrix.dot(RGB)*1023.0
    RGB  = np.round(RGB)
    R = max(min(int(RGB[0]), 1023), 0)
    G = max(min(int(RGB[1]), 1023), 0)
    B = max(min(int(RGB[2]), 1023), 0)
    print("gamut、cct RGB: ", R, G, B)
    
    t = 1/2.1
    R = pow(R/1023,t)*1023
    G = pow(G/1023,t)*1023
    B = pow(B/1023,t)*1023
    
    #1d lut
    R = one_d_lut[R][0]
    G = one_d_lut[G][1]
    B = one_d_lut[B][2]
    XYZ = RGB_to_XYZ(M, [R,G,B], 2.4, XYZW[1])
    print("1d_lut RGB: ", R, G, B)
    
    print("XYZ: ", XYZ)
    print("xy: ", XYZ[0]/sum(XYZ),  XYZ[1]/sum(XYZ))
    
    