make -s

run()
{
	start=$(date +%s%N)
	printf "line 1\nline 2" | ./"$1"
	status=$?
	end=$(date +%s%N)
	printf '\n%24s%7d ms\n' 'Total time:' $(( (end - start) / 1000000 ))
	return $status
}

ok=1
run gnl_1 || ok=0
printf '\n'
run gnl_42 || ok=0
printf '\n'
run gnl_8192 || ok=0
printf '\n'
run gnl_9999 || ok=0
printf '\n'
run gnl_10000000 || ok=0
printf '\n'
run gnl_0
if [ $? -eq 1 ]; then
	printf '\nBUFFER_SIZE=0 fails as expected: get_next_line always returns NULL\n'
else
	printf '\nBUFFER_SIZE=0 should have failed: get_next_line should return NULL\n'
	ok=0
fi

if [ $ok -eq 1 ]; then
	cat <<'EOF'


       ::::::::  :::    ::: 
     :+:    :+: :+:   :+:   
    +:+    +:+ +:+  +:+     
   +#+    +:+ +#++:++       
  +#+    +#+ +#+  +#+       
 #+#    #+# #+#   #+#       
 ########  ###    ###       

EOF
else
	exit 1
fi
