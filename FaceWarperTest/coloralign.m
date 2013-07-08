%load('data1.txt');
%load('data2.txt');
data1 = [5*randn(1000,1) 3*randn(1000,1)-2 2*randn(1000,1)+3];
data2 = [5*randn(1000,1)-4 2*randn(1000,1)+2 1*randn(1000,1)-1];
m1 = mean(data1);
m2 = mean(data2);
S1 = cov(data1);
S2 = cov(data2);
[vct1 val1] = eig(S1);
[vct2 val2] = eig(S2);
[vct1,val1]=sortem(vct1,val1);
[vct2,val2]=sortem(vct2,val2);
%vct2(:,3) = -vct2(:,3);
M1 = ones(size(data1,1), 1)*m1;
M2 = ones(size(data1,1), 1)*m2;
tmp1=(data1-M1)* vct1;
%tmp2 = vct2'*M2;
S =sqrt(diag(diag(val2)./diag(val1)));
tmp2 = tmp1*S;
tmp3 = tmp2*vct2;
tmp4 = tmp3+M2;
figure(1);
axis image
plot3(data2(:,1), data2(:,2), data2(:,3), 'r.', data1(:,1), data1(:,2), data1(:,3), 'b.');
figure(2);
plot3(data2(:,1), data2(:,2), data2(:,3), 'r.', tmp4(:,1), tmp4(:,2), tmp4(:,3), 'b.');
S3 = cov(tmp4);
[vct3 val3] = eig(S3);
 
tdata = [data1 ones(size(data1,1),1)];
T1 = [1 0 0 -m1(1);0 1 0 -m1(2);0 0 1 -m1(3);0 0 0 1];
R1 = eye(4, 4);
R1(1:3,1:3) = vct1';
S =sqrt(diag([diag(val2);1]./[diag(val1);1]));
R2 = eye(4, 4);
R2(1:3,1:3) = vct2';
T2 = [1 0 0 m2(1); 0 1 0 m2(2); 0 0 1 m2(3);0 0 0 1];
T = T2*R2*S*R1*T1;
ttt1 = (R1*T1*tdata');
ttt2 = S*ttt1;
ttt3 = R2*ttt2;
ttt4 = T2*ttt3;
ttt = (T*tdata')';
S4 = cov(ttt(:, 1:3));
[vct4 val4] = eig(S4);
vct1(:,3)'*cross(vct1(:,1),vct1(:,2))
vct2(:,3)'*cross(vct2(:,1),vct2(:,2))
