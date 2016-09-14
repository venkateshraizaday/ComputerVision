% Trying to cluster all images together irrespective of which video or sequence they belong
% this approach does not cluster the images as intended
taskDirs = dir('C:\Spring2016\CompVision\Project\sbairagi-arkhande-vraizada-final\images\cleanandjerk\*');
count = 1;
for index = 1 : length(taskDirs);
    imageSubDirPath = strcat('C:\Spring2016\CompVision\Project\sbairagi-arkhande-vraizada-final\images\cleanandjerk\',taskDirs(index).name);
    imageFiles = dir(strcat(imageSubDirPath,'\*.jpg'));
    numOfImages = length(imageFiles);
    if index == 15
        disp(imgPath)
        for k = 1:numOfImages
            imgPath = strcat(imageSubDirPath,'\',imageFiles(k).name);
            img = imread(imgPath); 
            [featureVector, hogVisualization] = extractHOGFeatures(img);
            featureVectorArray(k, :) = featureVector;
        
        end
    end
   
    if index == 15 
        disp(imgPath)
        break;
    end
end

[idx, C] = kmeans(featureVectorArray, 4);