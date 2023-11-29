import numpy as np

def convert_impulses2_xy(imp_R, imp_L, r, L, total_rotations, alpha):
    perimeter = 2 * np.pi * r

    # in radians
    rad_R = (imp_R/total_rotations) * 2 * np.pi
    rad_L = (imp_L/total_rotations) * 2 * np.pi

    alpha_new = ((rad_R - rad_L)*r)/L
    alpha += alpha_new

    ds_Ri = imp_R/total_rotations * perimeter
    ds_Li = imp_L/total_rotations * perimeter

    Tri = (ds_Ri + ds_Li) / 2  # to jest przemieszczenie
    # Rri = (ds_Ri + ds_Li) / L
    #
    # Kri = Rri/Tri
    # sri = 0 + Tri

    x_mov = Tri * np.cos(alpha)
    y_mov = Tri * np.sin(alpha)

    return x_mov, y_mov, alpha

# total_rotations = 600 # = 2*pi rad
# r = 1
# L = 1
# perimeter = 2*np.pi*r
#
# alpha = np.pi
#
# imp_R = 20
# imp_L = 0
#
# x, y, alpha = convert_impulses2_xy(imp_R, imp_L, r, L, total_rotations, alpha)
#
# print(x)
# print(y)
# print(alpha)