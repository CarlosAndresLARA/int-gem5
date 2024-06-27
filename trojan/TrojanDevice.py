from m5.params import *
from m5.objects.RealView import AmbaDmaDevice
from m5.proxy import *
          
class TrojanDevice(AmbaDmaDevice):

    type = 'TrojanDevice'
    cxx_header = "trojan/trojan_device.hh"
    cxx_class = 'gem5::TrojanDevice'
    
    
    
