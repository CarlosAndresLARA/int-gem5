close all
clear
clc

%one byte ps
fs = 100e6;

c2 = load('osc/C2--7750_11400--00000.dat');
c3 = load('osc/C3--7750_11400--00000.dat');
c4 = load('osc/C4--7750_11400--00000.dat');

c2f = (c2 > 1); %trigger
c3f = movavg(c3, 'simple', 10e4); %VCCIN
c4f = movavg(c4, 'simple', 10e4); %VCCOUT

figure
plot(c3f)
hold on
plot(c2f*1e-2-4e-3)
xlim([0 16e6])
ylim([-0.005 0.009])
grid on
box off
xlabel('Samples')
ylabel('Voltage (filtered)')
legend('V_{IN}','Trigger')

figure
plot(c4f)
hold on
plot(c2f*3e-3-15e-4)
xlim([0 16e6])
ylim([-0.002 0.003])
grid on
box off
xlabel('Samples')
ylabel('Voltage (filtered)')
legend('V_{OUT}','Trigger')