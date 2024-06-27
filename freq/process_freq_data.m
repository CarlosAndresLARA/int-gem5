close all
clear
clc

%attack1 cpu to cpu

colors = lines(2);

a1_data_real = load('osc/a1.dat');
a1_data_gem5 = load('m5out/a1.dat');

figure
stem(a1_data_real/1000)
xlim([8 108])
ylim([0 650])
grid on
box off
xlabel('Samples')
ylabel('Frequency (MHz)')

figure
stem(a1_data_gem5/1000, 'color', colors(2,:))
xlim([15 115])
ylim([0 650])
grid on
box off
xlabel('Samples')
ylabel('Frequency (MHz)')

%attack2 cpu to fpga, with driver access

a3_data_real = load('osc/a2.dat');
a3_data_gem5 = load('m5out/a2.dat');

a3_data_gem5 = a3_data_gem5(:,2) + a3_data_gem5(:,3);

figure
stem(a3_data_real(1323:2079))
xlim([0 300])
grid on
box off
xlabel('Samples')
ylabel('Sensor output')

figure
stem(a3_data_gem5(1357:1357+700), 'color', colors(2,:))
xlim([0 300])
grid on
box off
xlabel('Samples')
ylabel('Sensor output')

%attack3 cpu to fpga, with register access

a2_data_real = load('osc/a3.dat');
a2_data_gem5 = load('m5out/a3.dat');

a2_data_gem5_x = a2_data_gem5(:,2);
a2_data_gem5_x(a2_data_gem5_x == 400) = 0;
a2_data_gem5_x(a2_data_gem5_x == 750) = 400;
a2_data_gem5_x(a2_data_gem5_x == 0) = 750;
a2_data_gem5 = a2_data_gem5_x + a2_data_gem5(:,3);


figure
stem(a2_data_real(292:292+300))
xlim([0 300])
grid on
box off
xlabel('Samples')
ylabel('Sensor output')

figure
stem(a2_data_gem5, 'color', colors(2,:))
xlim([0 300])
grid on
box off
xlabel('Samples')
ylabel('Sensor output')