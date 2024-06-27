# Copyright (c) 2016-2017, 2019-2021 ARM Limited
# All rights reserved.
#
# The license below extends only to copyright in the software and shall
# not be construed as granting a license to any other intellectual
# property including but not limited to intellectual property relating
# to a hardware implementation of the functionality of the software
# licensed hereunder.  You may use the software subject to the license
# terms below provided that you ensure that this notice is replicated
# unmodified and in its entirety in all distributions of the software,
# modified or unmodified, in source code or in binary form.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# This is an example configuration script for full system simulation of
# a generic ARM bigLITTLE system.

import argparse
import os
import sys
import m5
import m5.util
import m5.debug
from m5.objects import *

m5.util.addToPath("../../")

from common import FSConfig
from common import SysPaths
from common import ObjectList
from common import Options

import zynqMP_devices as devices
from devices import AtomicCluster, KvmCluster, FastmodelCluster

import workloads

default_disk = 'aarch64-ubuntu-trusty-headless.img'

default_mem_size= "2GB"

def _to_ticks(value):
    """Helper function to convert a latency from string format to Ticks"""

    return m5.ticks.fromSeconds(m5.util.convert.anyToLatency(value))

def _using_pdes(root):
    """Determine if the simulator is using multiple parallel event queues"""

    for obj in root.descendants():
        if not m5.proxy.isproxy(obj.eventq_index) and \
               obj.eventq_index != root.eventq_index:
            return True

    return False

class ARMCluster(devices.CpuCluster):
    def __init__(self, system, num_cpus, cpu_clock,
                 cpu_voltage="1.0V"):
        cpu_config = [ ObjectList.cpu_list.get("MinorCPU"), devices.L1I,
            devices.L1D, devices.L2 ]
        super(ARMCluster, self).__init__(system, num_cpus, cpu_clock,
                                         cpu_voltage, *cpu_config)

def createSystem(caches, kernel, bootscript, machine_type="VExpress_GEM5",
                 disks=[],  mem_size=default_mem_size, bootloader=None):
    platform = ObjectList.platform_list.get(machine_type)
    m5.util.inform("Simulated platform: %s", platform.__name__)

    sys = devices.SimpleSystem(caches, mem_size, platform(),
                               workload=ArmFsLinux(
                                   object_file=SysPaths.binary(kernel)),
                               readfile=bootscript)

    sys.mem_ctrls = [ SimpleMemory(range=r, port=sys.membus.mem_side_ports)
                      for r in sys.mem_ranges ]

    sys.connect()

    # Attach disk images
    if disks:
        def cow_disk(image_file):
            image = CowDiskImage()
            image.child.image_file = SysPaths.disk(image_file)
            return image

        sys.disk_images = [ cow_disk(f) for f in disks ]
        sys.pci_vio_block = [ PciVirtIO(vio=VirtIOBlock(image=img))
                              for img in sys.disk_images ]
        for dev in sys.pci_vio_block:
            sys.attach_pci(dev)

    sys.realview.setupBootLoader(sys, SysPaths.binary, bootloader)

    return sys

cpu_types = {
    "atomic" : (AtomicCluster),
    "timing" : (ARMCluster),
}

# Only add the FastModel CPU if it has been compiled into gem5
if devices.have_fastmodel:
    cpu_types["fastmodel"] = (FastmodelCluster, FastmodelCluster)

def addOptions(parser):
    parser.add_argument("--restore-from", type=str, default=None,
                        help="Restore from checkpoint")
    parser.add_argument("--dtb", type=str, default=None,
                        help="DTB file to load")
    parser.add_argument("--kernel", type=str, required=True,
                        help="Linux kernel")
    parser.add_argument("--root", type=str, default="/dev/vda1",
                        help="Specify the kernel CLI root= argument")
    parser.add_argument("--machine-type", type=str,
                        choices=ObjectList.platform_list.get_names(),
                        default="VExpress_GEM5",
                        help="Hardware platform class")
    parser.add_argument("--disk", action="append", type=str, default=[],
                        help="Disks to instantiate")
    parser.add_argument("--bootscript", type=str, default="",
                        help="Linux bootscript")
    parser.add_argument("--cpu-type", type=str, choices=list(cpu_types.keys()),
                        default="timing",
                        help="CPU simulation mode. Default: %(default)s")
    parser.add_argument("--kernel-init", type=str, default="/sbin/init",
                        help="Override init")
    parser.add_argument("--cpus", type=int, default=1,
                        help="Number of CPUs to instantiate")
    parser.add_argument("--caches", action="store_true", default=False,
                        help="Instantiate caches")
    parser.add_argument("--last-cache-level", type=int, default=2,
                        help="Last level of caches (e.g. 3 for L3)")
    parser.add_argument("--cpu-clock", nargs="+", default="1GHz",
                        help="CPU clock frequency")
    parser.add_argument("--sim-quantum", type=str, default="1ms",
                        help="Simulation quantum for parallel simulation. " \
                        "Default: %(default)s")
    parser.add_argument("--mem-size", type=str, default=default_mem_size,
                        help="System memory size")
    parser.add_argument("--kernel-cmd", type=str, default=None,
                        help="Custom Linux kernel command")
    parser.add_argument("--bootloader", action="append",
                        help="executable file that runs before the --kernel")
    parser.add_argument("--kvm-userspace-gic", action="store_true",
                        default=False,
                        help="Use the gem5 GIC in a KVM simulation")
    parser.add_argument("-P", "--param", action="append", default=[],
        help="Set a SimObject parameter relative to the root node. "
             "An extended Python multi range slicing syntax can be used "
             "for arrays. For example: "
             "'system.cpu[0,1,3:8:2].max_insts_all_threads = 42' "
             "sets max_insts_all_threads for cpus 0, 1, 3, 5 and 7 "
             "Direct parameters of the root object are not accessible, "
             "only parameters of its children.")
    parser.add_argument("--vio-9p", action="store_true",
                        help=Options.vio_9p_help)
    parser.add_argument("--dtb-gen", action="store_true",
                        help="Doesn't run simulation, it generates a DTB only")
    parser.add_argument("--cpu-voltage", nargs="+", default="1.0V", 
                         help="CPU voltage(s).")
    parser.add_argument("--dvfs", action="store_true", help="Enable the DVFS Handler.")
    parser.add_argument("--flags", metavar="FLAG[,FLAG]", type=str,
                        help="flags for the debug sections")  
    parser.add_argument("--power-models", action="store_true",
                        help="Add power models to the simulated system. "
                             "Requires using the 'timing' CPU."
    )
    parser.add_argument("--stat-freq", type=float, default=1.0,
                        help="Frequency (in seconds) to dump stats to the "
                             "'stats.txt' file. Supports scientific notation, "
                             "e.g. '1.0E-3' for milliseconds."
    )
    parser.add_argument("--workload", type=str,
                        default="ArmBaremetal",
                        choices=workloads.workload_list.get_names(),
                        help="Workload type")
    return parser

def build(options):
    m5.ticks.fixGlobalFrequency()

    kernel_cmd = [
        "earlyprintk",
        "earlycon=pl011,0x1c090000",
        "console=ttyAMA0",
        "lpj=19988480",
        "norandmaps",
        "loglevel=8",
        "mem=%s" % options.mem_size,
        "root=%s" % options.root,
        "rw",
        "init=%s" % options.kernel_init,
        "vmalloc=768MB",
    ]

    root = Root(full_system=True)

    disks = [default_disk] if len(options.disk) == 0 else options.disk
    system = createSystem(options.caches,
                          options.kernel,
                          options.bootscript,
                          options.machine_type,
                          disks=disks,
                          mem_size=options.mem_size,
                          bootloader=options.bootloader)

    root.system = system
    if options.kernel_cmd:
        system.workload.command_line = options.kernel_cmd
    else:
        system.workload.command_line = " ".join(kernel_cmd)

    arm_model = cpu_types[options.cpu_type]


    system.armCluster = arm_model(system, options.cpus, options.cpu_clock, options.cpu_voltage)
    system.mem_mode = system.armCluster.memoryMode()

    # create caches
    system.addCaches(options.caches, options.last_cache_level)
    if not options.caches:
        if options.cpus > 0 and system.armCluster.requireCaches():
            m5.util.panic("ARM CPU model requires caches")

    # Linux device tree
    if options.dtb is not None:
        system.workload.dtb_filename = SysPaths.binary(options.dtb)
    else:
        system.workload.dtb_filename = \
            os.path.join(m5.options.outdir, 'system.dtb')
        system.generateDtb(system.workload.dtb_filename)

    if options.vio_9p:
        FSConfig.attach_9p(system.realview, system.iobus)

    if options.dvfs:
        system.dvfs_handler.domains = [system.armCluster.clk_domain]
        system.dvfs_handler.enable = options.dvfs
        system.dvfs_handler.transition_latency = '600ns'
    
    system.trojan = TrojanDevice(pio_addr=0x2c200000,interrupt=ArmSPI(num=119),amba_id=0x00142254)
    system.trojan.pio = system.membus.master
    system.trojan.dma = system.membus.slave 
    system.realview.dcc.osc_trojanDevice = CoreTile2A15DCC.Osc(device=9, freq="35MHz")
    system.trojan.clk_domain = system.realview.dcc.osc_trojanDevice 

    if options.flags:
        flags = options.flags.split(",")
        for flag in flags:
            if flag not in m5.debug.flags:
                print("invalid debug flag '%s'" % flag, file=sys.stderr)
                continue
            else:
                m5.debug.flags[flag].enable()
                m5.util.inform("%s debug flag has been enabled.", flag)

    return root

def instantiate(options, checkpoint_dir=None):
    # Setup the simulation quantum if we are running in PDES-mode
    # (e.g., when using KVM)
    root = Root.getInstance()
    if root and _using_pdes(root):
        m5.util.inform("Running in PDES mode with a %s simulation quantum.",
                       options.sim_quantum)
        root.sim_quantum = _to_ticks(options.sim_quantum)

    # Get and load from the chkpt or simpoint checkpoint
    if options.restore_from:
        if checkpoint_dir and not os.path.isabs(options.restore_from):
            cpt = os.path.join(checkpoint_dir, options.restore_from)
        else:
            cpt = options.restore_from

        m5.util.inform("Restoring from checkpoint %s", cpt)
        m5.instantiate(cpt)
    else:
        m5.instantiate()


def run(checkpoint_dir=m5.options.outdir):
    # start simulation (and drop checkpoints when requested)
    while True:
        event = m5.simulate()
        exit_msg = event.getCause()
        if exit_msg == "checkpoint":
            print("Dropping checkpoint at tick %d" % m5.curTick())
            cpt_dir = os.path.join(checkpoint_dir, "cpt.%d" % m5.curTick())
            m5.checkpoint(cpt_dir)
            print("Checkpoint done.")
        else:
            print(exit_msg, " @ ", m5.curTick())
            break

    sys.exit(event.getCode())


def generateDtb(root):
    root.system.generateDtb(os.path.join(m5.options.outdir, "system.dtb"))


def main():
    parser = argparse.ArgumentParser(
        description="ZynqMP ARM-system configuration")
    addOptions(parser)
    options = parser.parse_args()
    root = build(options)
    root.apply_config(options.param)
    instantiate(options)
    m5.stats.periodicStatDump(m5.ticks.fromSeconds(options.stat_freq))
    if options.dtb_gen:
      generateDtb(root)
    else:
      run()


if __name__ == "__m5_main__":
    main()
