% Trying to cluster frame sequences into groups from each vide0
% the expected outcome was supposed to group similar frame number together. Like all first frames are grouped together, second frames , third frames and so on
% The outcome was not useful since again frames from one video are grouped together due to background clutter
% images of same folder were grouped into same cluster
taskDirs = dir('C:\Spring2016\CompVision\Project\sbairagi-arkhande-vraizada-final\images\cleanandjerk\*');
count = 1;
for index = 1 : length(taskDirs);
    imageSubDirPath = strcat('C:\Spring2016\CompVision\Project\sbairagi-arkhande-vraizada-final\images\cleanandjerk\',taskDirs(index).name);
    imageFiles = dir(strcat(imageSubDirPath,'\*.jpg'));
    numOfImages = length(imageFiles);
    for k = 1:numOfImages
        %imgPath = strcat('C:\Spring2016\CompVision\Project\sbairagi-arkhande-vraizada-final\train\javelinthrow\JavelinThrow1\',imageFiles(k).name);
        imgPath = strcat(imageSubDirPath,'\',imageFiles(k).name);
        img = imread(imgPath); 
       [featureVector, hogVisualization] = extractHOGFeatures(rgb2gray(img), 'CellSize', [4 4]);
       featureVectorArray(count , :) = featureVector;
        %featureVector = detectSURFFeatures(rgb2gray(img));
        %featureVectorArray(count , :) = extractLBPFeatures(rgb2gray(img));
        count = count + 1;
    %     figure;
    %     imshow(img); hold on;
    %     plot(hogVisualization);
    %     %break;
    end
end

[idx, C] = kmeans(featureVectorArray, 7);