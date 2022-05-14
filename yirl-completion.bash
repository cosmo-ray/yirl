
_yirl()
{
    prev=${COMP_WORDS[COMP_CWORD-1]}
    cur=${COMP_WORDS[COMP_CWORD]}

    if [ "--arg" == $prev -o "--height" == $prev -o "--name" == $prev -o "--width" == $prev ]; then
	return
    elif [ "--binary-root-path" == $prev -o "--start-dir" ==  $prev ]; then
	COMPREPLY=($(compgen -o  dirnames -- "${cur}"))
	return
    fi
    COMPREPLY=($(compgen -W "--name --width --height --arg --linux-user-path --binary-root-path --start-dir" -- "${cur}"))
}

complete  -F _yirl yirl-loader
