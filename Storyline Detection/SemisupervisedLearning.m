%%
clear
taskDirs = dir('C:\Spring2016\CompVision\Project\sbairagi-arkhande-vraizada-final\train\cleanandjerk\*');
count = 1;
%%
for index = 1 : length(taskDirs);
    imageSubDirPath = strcat('C:\Spring2016\CompVision\Project\sbairagi-arkhande-vraizada-final\train\cleanandjerk\', int2str(index));
    imageFiles = dir(strcat(imageSubDirPath,'\*.jpg'));
    numOfImages = length(imageFiles);
    if numOfImages < 1
        continue
    else
    for k = 1:numOfImages
        imgPath = strcat(imageSubDirPath,'\',imageFiles(k).name);
        img = imread(imgPath); 
        img = imresize(img, [32 32]);
       [featureVector1, hogVisualization] = extractHOGFeatures(rgb2gray(img), 'CellSize', [4 4]);
       [featureVector2, hogVisualization] = extractHOGFeatures(rgb2gray(img), 'CellSize', [8 8]);
        featureVectorArray(count , : ) = horzcat(featureVector1,featureVector2);
        count = count + 1;
    end
    meanFeatureVecArray(index, :) = mean(featureVectorArray);
    %disp(index);
    %disp(imageSubDirPath);
    clear featureVectorArray
    end 
end

%%
img = imread('C:\Spring2016\CompVision\Project\sbairagi-arkhande-vraizada-final\images\cleanandjerk\CleanAndJerk10\frame96.jpg'); 
img = imresize(img, [32 32]);
[featureVector1, hogVisualization] = extractHOGFeatures(rgb2gray(img), 'CellSize', [4 4]);
[featureVector2, hogVisualization] = extractHOGFeatures(rgb2gray(img), 'CellSize', [8 8]);
featureVector = horzcat(featureVector1,featureVector2);
closestMatch = 1000;
minDistance = 100000;
for index = 1 : size(meanFeatureVecArray,1);
    
    distValue = pdist2(featureVector, meanFeatureVecArray(index, :));
    disp(distValue);
    if(distValue < minDistance)
        minDistance = distValue;
        closestMatch = index;
    end
end
disp('Closest Match ')
disp(int2str(closestMatch))
    
    
