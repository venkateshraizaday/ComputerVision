function K = hist_intersection(A, B)
    a = size(A,2); b = size(B,2); 
    K = zeros(a, b);
    for i = 1:a
      Va = repmat(A(:,i),1,b);
      K(i,:) = 0.5*sum(Va + B - abs(Va - B));
    end
end