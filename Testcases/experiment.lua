function f(a)
	print(a);
	if a < 10 then
		f(a+1)
	end
end

f(2)