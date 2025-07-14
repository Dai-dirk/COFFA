# COFFA-Architecture
=======================

RISC-V + FGRA modeling and generation with Chisel.

## Dependencies

**Chipyard 1.10.0:** please refer to chipyard official documentation https://chipyard.readthedocs.io/en/latest/Chipyard-Basics/Initial-Repo-Setup.html

**JDK 8 or newer**

**SBT**

## File catalog

### Source codes
**./src/main/scala:** the source codes of FGRA and other control modules

**./src/main/scala/soc/COFFA.scala:** the top file

**./src/main/scala/dsa/:** the source codes of the FGRA array, including FPE, FGIB, and IOB.     

### input files
**./src/main/resources/fgra_spec.json:** the specification file of FGRA that describes the design parameters

**./src/main/resources/fgra_adg.json:** the generated ADF of FGRA

**./src/main/resources/operation.json:** the file that describes what operations are supported by the generated FGRA

              


