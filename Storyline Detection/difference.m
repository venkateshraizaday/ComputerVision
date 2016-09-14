% Code from this point has been taken from C. Panagiotakis, N. Ovsepian and E. Michael, Video Synopsis based on a Sequential Distortion Minimization Method, International Conference on Computer Analysis of Images and Patterns, 2013.

function [ r ] = difference( f1,f2 )
       k=rgb2gray(f1);  % Convert into gray scale
       l=rgb2gray(f2);
       f11=imhist(k);  %histogram of data
       f12=imhist(l);
       diffe=imabsdiff(f11,f12); %Absolute difference of two images
       r=sum(diffe);
   end