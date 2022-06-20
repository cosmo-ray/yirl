_yirl()
{
    prev=${COMP_WORDS[COMP_CWORD-1]}
    cur=${COMP_WORDS[COMP_CWORD]}

    if [ "--arg" == $prev -o "--height" == $prev -o "--name" == $prev -o "--width" == $prev ]; then
	return
    elif [ "--binary-root-path" == $prev -o "--start-dir" ==  $prev -o\
	   '-d' == $prev -o '-P' == $prev -o\
	   '--linux-user-path' == $prev ]; then
	COMPREPLY=($(compgen -o  dirnames -- "${cur}"))
	return
    fi
    COMPREPLY=($(compgen -W "--help --name --width --height --arg --linux-user-path --binary-root-path --start-dir" -- "${cur}"))
}

complete  -F _yirl yirl-loader
complete  -F _yirl ./yirl-loader.sh
