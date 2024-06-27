close all
clear
clc

cl = lines(6);

clk = load('m5out/stats/system.armCluster.clk_domain.clock.csv');
clk_t = load('m5out/stats_t/system.armCluster.clk_domain.clock.csv');
n = clk_t(end);
s = nan(1, n);
a = s;
a(clk_t) = clk;

ts = 1e-6;
t = (1:n)*ts;

figure
hold on
plot(nan,'Color',cl(4,:),'LineWidth',2)
plot(nan,'Color',cl(5,:),'LineWidth',2)
plot(nan,'Color',cl(3,:),'LineWidth',2)

v = load('m5out/stats/system.armCluster.voltage_domain.voltage.csv');
v_t = load('m5out/stats_t/system.armCluster.voltage_domain.voltage.csv');
w = s;
w(v_t) = v;

ipc0 = load('m5out/stats/system.armCluster.cpus0.ipc.csv');
ipc0_t = load('m5out/stats_t/system.armCluster.cpus0.ipc.csv');
x = s;
x(ipc0_t) = ipc0;

plot(t, x/max(x),'.','Color',cl(4,:))

miss0 = load('m5out/stats/system.armCluster.cpus0.dcache.overallMisses::total.csv');
miss0_t = load('m5out/stats_t/system.armCluster.cpus0.dcache.overallMisses::total.csv');
y = s;
y(miss0_t) = miss0;

plot(t, y/max(y),'.','Color',cl(5,:))

hit0 = load('m5out/stats/system.armCluster.cpus0.committedInstType_0::IntAlu.csv');
hit0_t = load('m5out/stats_t/system.armCluster.cpus0.committedInstType_0::IntAlu.csv');
z = s;
z(hit0_t) = hit0;

plot(t, z/max(z),'.','Color',cl(3,:))
box off
grid on
xlabel('Time (s)')
ylabel('Normalized amplitude')
legend('core IPC', 'd-cache overalMisses', 'committedInst IntAlu')

p = w .* (2*x + 3e-9*y);
figure
hold on
plot(t, p/max(p),'.','Color',cl(2,:))
box off
grid on
xlabel('Time (s)')
ylabel('Normalized amplitude')
