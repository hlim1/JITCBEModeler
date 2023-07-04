# JITCBEModeler

Just-in-Time Compiler Back End Modeler (JITCBEModeler)

## Publication

HeuiChan Lim and Saumya Debray. 2023. Automatically Localizing Dynamic Code Generation Bugs in JIT Compiler Back-End. In Proceedings of the 32nd ACM SIGPLAN International Conference on Compiler Construction (CC 2023). Association for Computing Machinery, New York, NY, USA, 145–155. https://doi.org/10.1145/3578360.3580260

## Requirements:
### Linux
- Intel's [Pin Tool](https://www.intel.com/content/www/us/en/developer/articles/tool/pin-a-binary-instrumentation-tool-downloads.html)
    - Stable Version for BackEndModeler is 3.13.
    - Set the environment variable `PIN_ROOT` to point to the directory containing Pin.
- Intel's [XED](https://intelxed.github.io/) disassembler.
    - Set the environment variable `XED_ROOT` to point to the directory containing xed.

### Windows
- Not Supported.

### MacOS
- Not Supported.

## BackEndModeler Execute Command and Example

```
<PIN_ROOT>/pin -t <PIN_ROOT>/JITCBEModeler/BackEndModeler/obj-intel64/BackEndModeler.so -- <Target Executable> <Input Program to the Target Executable>

e.g.,
  <PIN_ROOT>/pin -t <PIN_ROOT>/JITCBEModeler/BackEndModeler/IRModeler/obj-intel64/BackEndModeler.so -- d8 poc.js
```

## Output
    Formated JSON file - backend.json
