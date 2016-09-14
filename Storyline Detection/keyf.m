% Code from this point has been taken from C. Panagiotakis, N. Ovsepian and E. Michael, Video Synopsis based on a Sequential Distortion Minimization Method, International Conference on Computer Analysis of Images and Patterns, 2013.
function keyf(V,path)
%V = 'v_CleanAndJerk_g03_c03.avi';      %Video Name 
xyloObj = VideoReader(V);   %Using video reader reading video

   %Extracting frames
   T= xyloObj.NumberOfFrames            % Calculating number of frames
   for g=1:T
           p=read( xyloObj,g);          % Retrieve data from video
           if(g~=  xyloObj.NumberOfFrames)
                 J=read( xyloObj,g+1);
                 th=difference(p,J);        %To calculate histogram difference between two frames 
                 X(g)=th;
           end
   end
   temp = sort(X,'descend');
   frame_num = 1;
   %calculating mean and standard deviation and extracting frames
   %mean=mean2(X)
   %std=std2(X)
   threshold=temp(9);
   for g=1: T
       p =  read(xyloObj,g);
       if(g~=xyloObj.NumberOfFrames)
        J=   read(xyloObj,g+1);
        th=difference(p,J);
        if(th>threshold)    % Greater than threshold select as a key frame
            
filename = fullfile(path, sprintf('%d.JPG', frame_num));   %Writing the keyframes
imwrite(J, filename);
         frame_num = frame_num + 1;
                
       end 
       end
   end 
end