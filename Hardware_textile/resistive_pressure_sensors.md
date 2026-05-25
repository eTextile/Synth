# Resistive Pressure Sensors

Resistive pressure sensors measure the magnitude of an applied pressure and/or detect contact. By extension, they can also determine a linear or surface position. These sensors are widely used in industrial applications owing to their low manufacturing cost: they are typically produced by printing functional inks onto flexible plastic films.

A resistive pressure sensor comprises three layers of functional inks — conductive / resistive / conductive — separated by one or two spacer layers. Antistatic packaging materials such as Velostat are commonly used as the resistive layer. When pressure is applied to the stack, the contact area between layers increases, which lowers the overall resistance of the sensor. This behaviour is governed by the parallel resistor law:

```
R = (R1 × R2) / (R1 + R2)
```

The greater the pressure, the more parallel conduction paths are created, and the lower the resulting resistance.

The cross-section below illustrates the deformation under a localised load (top) and a distributed load (bottom):

```
               |
               v
  -------- \ ____ / --------  ← conductive layer
  -  -  -  -  -  -  -  -  -  ← spacer
  ||||||||||||||||||||||||||  ← resistive layer
  --------------------------  ← conductive layer
```

```
           |   |   |
           v   v   v
  ----- \ ___________ / ----  ← conductive layer
  -  -  -  -  -  -  -  -  -  ← spacer
  ||||||||||||||||||||||||||  ← resistive layer
  --------------------------  ← conductive layer
```

---

# Piezo-Resistive Pressure Sensors

Like resistive sensors, piezo-resistive pressure sensors measure pressure, detect contact, and resolve position — both linear and surface. The key distinction is the resistance-change mechanism: instead of varying the contact area between separate layers, these sensors exploit the bulk compression of a piezo-resistive material.

Piezo-resistive coatings are combined with highly resilient substrates such as open-cell foams or nonwoven fabrics, including anti-static packaging foams commonly used to protect electronic components. Fibres with analogous properties also exist. Under compression the material's resistance decreases, making this type of sensor behave similarly to a variable resistor (potentiometer).

```
  ----------------------------------------------------------------  ← conductive layer
```
<div style="display:flex;align-items:center;gap:12px;"><img src="./docs/piezo_resistive.png" alt="Piezo-resistive layer"><span>← piezo-resistive layer</span></div>
```
  ----------------------------------------------------------------  ← conductive layer
```

---

# Electrical Characteristics

The resistance of both resistive and piezo-resistive sensors varies in proportion to the applied pressure. In the resistive case, the variation stems from the change in inter-layer contact area; in the piezo-resistive case, it results from the compression of the bulk material. In both cases:

- **Maximum resistance** → sensor at rest (no load applied)
- **Minimum resistance** → sensor under full load

To read these sensors with a microcontroller, a **voltage divider** must be constructed by placing a fixed resistor in series with the sensor. The value of this reference resistor determines the operating range of the output voltage and should be chosen to match the sensor's nominal resistance range.

---

# Surface Pressure Sensors

Tracking the position of one or more contacts on a resistive or piezo-resistive sensor surface requires careful co-design of several elements:

- **Row and column layout** — geometry, pitch, and number of electrodes
- **Electrical interconnection** — routing between the textile matrix and the PCB
- **Signal processing** — image analysis algorithms for multi-touch blob detection and tracking
- **Inter-digitisation** — controlled overlap between row and column electrodes to improve spatial resolution
