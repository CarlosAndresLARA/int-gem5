# Source codes and binaries for the emulation of power and frequency intrinsic channels in gem5.

*Warning: there is likely to be many unmet dependencies.*

**1. Clone the repository:**

```
git clone https://github.com/CarlosAndresLARA/dvfs-gem5.git
cd dvfs-gem5
```

**2. Get/build the simulation sources (warning: requires root privileges):**

```
make
```

**3. Obtain a copy of gem5, modify it, and build it:**

```
git clone https://gem5.googlesource.com/public/gem5 -b v21.2.1.0
cp -r trojan/ gem5/src/
cp *.py gem5/configs/example/arm/
cd gem5/
scons -j $(nproc) build/ARM/gem5.opt
make -C util/term/
```

**4. Launch the simulation:**

```
./build/ARM/gem5.opt \
./configs/example/arm/zynqMP_mc.py \
--cpus 2 \
--cpu-type=timing \
--caches \
--machine-type=VExpress_GEM5_Foundation \
--bootloader=../binaries/binaries/boot_v2.arm64 \
--kernel=../vmlinux \
--disk=../binaries/ubuntu-18.04-arm64-docker.img \
--dvfs \
--cpu-clock 1200MHz 600MHz 400MHz 300MHz \
--cpu-voltage 0.9V 0.87V 0.86V 0.85V \
--last-cache-level 2 \
--flags=Trojan
```

**5. Connect to the simulation (in a new terminal):**

```
./util/term/m5term 3456
```

**6. Try the attack emulation:**

*CPU to CPU*

```
taskset -c 0 ./apps/attack1/send 1000 & taskset -c 1 ./apps/attack1/rec &
cat *.txt
```

*CPU to FPGA: driver*

```
./apps/attack2/send 0x2c200000 0x1C010000 0x154786fa 11000000
```

*CPU to FPGA: register*

```
./apps/attack3/send 0x2c200000 0x1C010000 0x154786fa 6000000
```

**7. Power covert channels**

*Create a checkpoint*

```
./build/ARM/gem5.opt \
./configs/example/arm/zynqMP_mc.py \
--cpus 2 \
--cpu-type=timing \
--caches \
--machine-type=VExpress_GEM5_Foundation \
--bootloader=../binaries/binaries/boot_v2.arm64 \
--kernel=../vmlinux \
--disk=../binaries/ubuntu-18.04-arm64-docker.img \
--bootscript=../scripts/bash.rcS \
--dvfs \
--cpu-clock 1200MHz 600MHz 400MHz 300MHz \
--cpu-voltage 0.9V 0.87V 0.86V 0.85V \
--last-cache-level 2 \
--flags=Trojan
```

*Increase stat rate to 1MSpS and restore from checkpoint*

```
./build/ARM/gem5.opt \
./configs/example/arm/zynqMP_mc.py \
--cpus 2 \
--cpu-type=timing \
--caches \
--machine-type=VExpress_GEM5_Foundation \
--bootloader=../binaries/binaries/boot_v2.arm64 \
--kernel=../vmlinux \
--disk=../binaries/ubuntu-18.04-arm64-docker.img \
--bootscript=../scripts/bash.rcS \
--dvfs \
--cpu-clock 1200MHz 600MHz 400MHz 300MHz \
--cpu-voltage 0.9V 0.87V 0.86V 0.85V \
--last-cache-level 2 \
--stat-freq 1.0E-6 \
--restore-from=./m5out/cpt.*
```

