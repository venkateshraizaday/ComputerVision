videos = dir('SoccerPenalty\*.avi');
num_videos = length(videos);
for i=1:num_videos
   currentfilename = videos(i).name;
   path = strcat('SoccerPenalty',int2str(i));
   path1 = strcat('SoccerPenalty\',path);
   mkdir(path1)
   keyf(strcat('SoccerPenalty\',currentfilename),path1);
end