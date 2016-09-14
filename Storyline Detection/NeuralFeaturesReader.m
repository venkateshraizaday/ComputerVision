clear;
clc;
fid = fopen('neural_features.txt');
lineNumber = 1;
tline = fgets(fid);
disp(tline);
while ischar(tline)
    %disp(tline)
   
    tline2 = fgets(fid);
    featureVector(lineNumber, : ) = str2num(tline2);
    tline3 = fgets(fid);
    disp(tline3);
    tline = fgets(fid);
    disp(tline);
 
    %imageNames(lineNumber, : ) = tline3;
    lineNumber = lineNumber + 1;    
end

fclose(fid);

[idx, C] = kmeans(featureVectorArray, 8);