"""Independent check of the published constants used by RWD COLOR LAB.
Implements each curve straight from the source documents/reference code and compares
against documented check values, then compares primaries-derived matrices with
matrices printed in the DJI and Panasonic documents. Run: python3 -I verify_specs.py"""
import numpy as np
from math import log10, log, exp
ok = True
def chk(name, got, want, tol):
    global ok
    good = abs(got - want) <= tol
    ok &= good
    print(("ok   " if good else "FAIL ") + f"{name}: got {got:.7f} want {want:.7f}")

# --- Sony S-Log3 (Sony technical summary, copy at wikis.utexas.edu)
def slog3(x): return (420+log10((x+0.01)/0.19)*261.5)/1023 if x>=0.01125 else (x*(171.2102946929-95)/0.01125+95)/1023
for r,c in [(0,95),(0.18,420),(0.9,598)]: chk(f"S-Log3 {r}->cv", slog3(r)*1023, c, 0.5)
# --- Sony S-Log2 (reference impl: S-Log applied to x/0.9*155/219, then full->legal)
def slog2(refl):
    x = refl/0.9*155/219
    y = (0.432699*log10(x+0.037584)+0.616596+0.03) if x>=0 else x*5+0.030001222851889303
    return (y*876+64)/1023
for r,c in [(0,90),(0.18,347),(0.9,582)]: chk(f"S-Log2 {r}->cv", slog2(r)*1023, c, 0.5)
# --- Fujifilm F-Log / F-Log2 (Fujifilm data sheet v1.0 constants)
def flog(x,a,b,c,d,e,f,cut1): return c*log10(a*x+b)+d if x>=cut1 else e*x+f
FL=(0.555556,0.009468,0.344676,0.790453,8.735631,0.092864,0.00089)
FL2=(5.555556,0.064829,0.245281,0.384316,8.799461,0.092864,0.000889)
for r,c in [(0,95),(0.18,470),(0.9,705)]: chk(f"F-Log {r}->cv", flog(r,*FL)*1023, c, 0.6)
for r,c in [(0,95),(0.18,400),(0.9,570)]: chk(f"F-Log2 {r}->cv", flog(r,*FL2)*1023, c, 0.6)
# --- Nikon N-Log (Nikon N-Log specification): x = 650*(y+0.0075)^(1/3) if y<0.328 else 150*ln(y)+619
def nlog(y): return (650*(y+0.0075)**(1/3) if y<0.328 else 150*log(y)+619)/1023
chk("N-Log 0.18->cv", nlog(0.18)*1023, 372.0, 0.6)   # Nikon doc: stop 0 at y=0.18; ~372 (derived)
chk("N-Log continuity @0.328", 650*(0.328+0.0075)**(1/3), 150*log(0.328)+619, 0.6)
# --- DJI D-Log (DJI D-Log/D-Gamut whitepaper)
def dlog(x): return 6.025*x+0.0929 if x<=0.0078 else log10(x*0.9892+0.0108)*0.256663+0.584555
for r,c in [(0,95),(0.18,408),(0.9,586)]: chk(f"D-Log {r}->cv", dlog(r)*1023, c, 0.6)
# --- Panasonic V-Log, Canon C-Log3 v1.2, BMD Film Gen5 (reference implementation, colour 0.4.7)
def vlog(x): return 5.6*x+0.125 if x<0.01 else 0.241514*log10(x+0.00873)+0.598206
for r,c in [(0,128),(0.18,433),(0.9,602)]: chk(f"V-Log {r}->cv", vlog(r)*1023, c, 0.6)
def clog3(x):
    x=x/0.9
    lo,hi=-0.014,0.014
    if x<lo: return -0.36726845*log10(-x*14.98325+1)+0.12783901
    if x<=hi: return 1.9754798*x+0.12512219
    return 0.36726845*log10(x*14.98325+1)+0.12240537
chk("C-Log3 0.18", clog3(0.18), 0.3433894, 2e-7)
def bmd5(x): return 8.283605932402494*x+0.09246575342465753 if x<0.005 else 0.08692876065491224*log(x+0.005494072432257808)+0.5300133392291939
chk("BMD Film Gen5 0.18", bmd5(0.18), 0.3835616, 2e-7)

# --- gamut matrices from primaries vs printed matrices
def rgb2xyz(prim, white):
    P=np.array([[x/y,1,(1-x-y)/y] for x,y in prim]).T
    W=np.array([white[0]/white[1],1,(1-white[0]-white[1])/white[1]])
    S=np.linalg.solve(P,W); return P*S
D65=(0.3127,0.3290)
dgamut=rgb2xyz([(0.71,0.31),(0.21,0.88),(0.09,-0.08)],D65)
pub=np.array([[0.6482,0.1940,0.1082],[0.2830,0.8132,-0.0962],[-0.0183,-0.0832,1.1903]])
d=np.abs(dgamut-pub).max(); ok&=d<2e-3; print(("ok   " if d<2e-3 else "FAIL ")+f"D-Gamut->XYZ max diff vs DJI printed matrix: {d:.5f}")
vg=rgb2xyz([(0.73,0.28),(0.165,0.84),(0.10,-0.03)],(0.31271,0.32902))
pubv=np.array([[0.679644,0.152211,0.1186],[0.260686,0.774894,-0.03558],[-0.00931,-0.004612,1.10298]])
d=np.abs(vg-pubv).max(); ok&=d<2e-3; print(("ok   " if d<2e-3 else "FAIL ")+f"V-Gamut->XYZ max diff vs Panasonic printed matrix: {d:.5f}")
# DaVinci Intermediate (Blackmagic DWG/DI document)
DI=lambda L:(np.log2(L+0.0075)+7)*0.07329248 if L>0.00262409 else L*10.44426855
chk("DaVinci Intermediate 0.18", DI(0.18), 0.336043, 1e-6)
print("ALL OK" if ok else "SOME FAILED")
