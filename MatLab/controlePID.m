d=0.18;
Ixy = 0.00896655;
Iz = 0.0152881;
m=0.696;

GPhiTheta = tf([0,d,0],[Ixy,0,0]);
GmPhiTheta = feedback(GPhiTheta,1);

GPsi = tf([0,d,0],[Iz,0,0]);
GmPsi = feedback(GPsi,1);

GZ = tf([0,1,0],[m*1,0,0]);
GmZ = feedback(GZ,1);

%amplitude em 15 graus
opt = stepDataOptions('InputOffset',0,'StepAmplitude',0.261799);
%pidtool(GmPhi)
step(GmPhiTheta,opt);
%step(GmPsi,opt);
%step(GmZ,opt);
%pidtool(GmZ)
