%code to segment the objects in the training image.
%the programm process as follows :
%-Binarization of the input image
%-compute the histogram of the pixel regarding the horizontal and vertical
%directions.
%compute the average of the average of black pixels in both horizontal and
%vertical directions, this will serve as a threshold to filter line which
%are not belonging tho squares
%compute the size of the obtained rectangle and elimitate those which size
%is different from the mean of the size.
%--------------------------------------------------------------------------
%the code starts here
clear all;
%the input file is named Sample0.png and is in the current directory
input_img = imread('Sample0.png');%reanding the input image
%reading the size of the image
%conversion to gray level and binarization
input_img = rgb2gray(input_img); %the gray image
%reading the size of the image
[M N] = size(input_img);
%convert the gray image to binary image
input_img = im2bw(input_img,graythresh(input_img));
%inversion of the bits because we want to count black pixels
input_img = ~input_img;
%imwrite(input_img,'binary_image.png','png');
%converting the binary image to double array to calculate histograms
input_img = im2double(input_img);
%calculate the histogram for both vertical and horizontal directions
v_hist = sum(input_img,1);
h_hist = sum(input_img,2);
%threshold for vertical and horizontal directions
percentage = 40;%percentage of the maximum for thresholding
v_thresh = max(v_hist)*(percentage/100); %average of the black pixels in the vertical direction
h_thresh = max(h_hist)*(percentage/100);%average of blac pixels in the horizontal direction
%we destroy the image for it is no more needed
 clear input_img;

%for demonstration purposes display both histograms
%======================================================
figure;
subplot(2,1,1);
plot(v_hist);
hold on
plot(1:1:N,v_thresh,'r');
title('histogram in the vertical direction');
subplot(2,1,2);
plot(h_hist);
hold on
title('histogram in the horizontal direction');
plot(1:1:M,h_thresh,'r');
%======================================================
%mask the values which are below the average to serve as first filter
v_mask = v_hist > v_thresh;
h_mask = h_hist > h_thresh;
%thresholding the bins of the histograms
v_hist = zeros(size(v_hist));
h_hist = zeros(size(h_hist));
v_hist(v_mask) = 1;
h_hist(h_mask) = 1;
%differentition of the thresholded vectors by substracting them to a
%shifted version of the same vector
v_hist_shift = [v_hist(2:length(v_hist)) v_hist(length(v_hist))];
h_hist_shift = [h_hist(2:length(h_hist)); h_hist(length(h_hist))];
v_diff = v_hist - v_hist_shift;
h_diff = h_hist - h_hist_shift;
h_diff = h_diff'; %transpose of h_diff
%for demonstration purposes display both histograms
%======================================================
% figure;
% subplot(2,1,1);
% bar(v_diff);
% title('derivative in the vertical direction');
% subplot(2,1,2);
% bar(h_diff);
% title('derivative in the horizontal direction');
%======================================================
%extraction of the lines' coordinates
v_vector = 1:1:N;
h_vector = 1:1:M;
%extraction of the lines'coordinates in both axes
v_lines_coord = v_vector(v_diff<0);
h_lines_coord = h_vector(h_diff<0);
%displaying lines on the original image
% figure; imshow('Sample0.png');
% hold on;
% %displaying vertical lines
% for n = 1: length(v_lines_coord)
% line([v_lines_coord(n) v_lines_coord(n)],[1 M],'Color','r','LineWidth',2)
% end
% %displaying horizontal lines
% for n = 1: length(h_lines_coord)
% line([1 N],[h_lines_coord(n) h_lines_coord(n)],'Color','b','LineWidth',2)
% end
%extraction of the coordinates for the retangle and filtering those who are
%to different from the average size regarding the diagonal of the rectangle
n_vlines = length(v_lines_coord);%number of lines in the vertical direction
n_hlines = length(h_lines_coord);%number of lines in the horizontal direction
rectangles = zeros((n_vlines-1)*(n_hlines-1),5);
k = 1;
for j = 1:n_hlines - 1 
    for i = 1 : n_vlines - 1
      rectangles(k,1) = v_lines_coord(i) + 10;%the x1 coordinate
      rectangles(k,2) = h_lines_coord(j)+10;%the y1 coordinate
      rectangles(k,3) = v_lines_coord(i+1)-5;%the x2 coordinate
      rectangles(k,4) = h_lines_coord(j+1)-5;%the y2 coordinate
      rectangles(k,5) = (v_lines_coord(i+1)- v_lines_coord(i))^2+(h_lines_coord(j+1) - h_lines_coord(j))^2;
      k = k +1;
    end
end
%filtering according to a value obtained by observing the samples
diag_threshold = 132000;
rect_mask = rectangles(:,5) > diag_threshold;
objects_coords = rectangles(rect_mask,:);
%drawing rectangles to select the objects
figure; imshow('Sample0.png');
hold on;
%displaying vertical lines
for n = 1: size(objects_coords,1)
rectangle('Position',[objects_coords(n,1), objects_coords(n,2), objects_coords(n,3)-objects_coords(n,1),...
     objects_coords(n,4) - objects_coords(n,2)],'edgecolor','r')
end






