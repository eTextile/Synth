# eTextile-Synthesizer
### Exploring music through textile
## HARDWARE_TEXTILE / HOWTO

![eTextile Synthesizer](https://farm1.staticflickr.com/798/40904680852_d5c9b1b35d_z_d.jpg)

The eTextile hardware has been designed to be easy to build with accessible and low-cost tools.
Here are the steps to make the conductive textile pattern on a woven cotton fabric.
First, apply a fusing material (hot-melt adhesive) to the conductive fabric.
Then use a CNC cutter to cut the two identical (X and Y) matrix patterns.
CNC cutters and vinyl plotters can be found at low cost on second-hand websites.
We recommend looking for an older model that uses a **parallel port** (not USB) — these are the most affordable.
A **parallel-to-USB adapter** lets you drive it from a modern computer using the Inkscape plugin below.
Once the conductive fabric layers are cut and bonded to the backing material, fuse them onto your woven fabric using a heat press.

### BOM (Bill of Materials)
- **Conductive fabric** : silver-coated polyester or nickel-coated polyester
- **CNC cutter** :
- **Heat press** :
- **Fusing material (hot-melt adhesive)** :

### CUTTING PATTERN
The E256 matrix uses an interdigitated zig-zag pattern with a built-in eTextile bus (pitch 4.2 mm).
Cutting file (TOP and BOTTOM antenna layers): [E256_path.svg](./E256_path.svg)

### SOFTWARE REQUIREMENTS
- [Inkscape](https://inkscape.org/)
- [Inkcut](https://github.com/codelv/inkcut)

#### Linux install
> **Note:** the commands below target Python 3 on a modern Debian/Ubuntu system.
```
sudo apt-get install python3-dev python3-pip python3-pyqt5

pip3 install twisted inkcut
```

## E256 — Cutting the conductive textile
Use the file above to cut the X and Y antenna layers of the E256 matrix in two separate passes.

## E256 — Fusing the conductive textile
Before fusing the conductive textile, we use a glue spray to fix it temporarily onto the textile and remove all the non-needed parts. After all, the functional part is placed in the press and fused to the fabric.
![Lost_parts](https://live.staticflickr.com/65535/48791432428_cccc5ecaf6_c_d.jpg)
![Heat_press](https://live.staticflickr.com/65535/48791938502_d166d7a995_c_d.jpg)

### TODO
- Add non-slip silicone backing
