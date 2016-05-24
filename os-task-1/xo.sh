#!/bin/bash

function initGame {
	#табица для игры

	a[1]=' '
	a[2]=' '
	a[3]=' '

	b[1]=' '
	b[2]=' '
	b[3]=' '

	c[1]=' '
	c[2]=' '
	c[3]=' '

	showTable
}

function showTable {
	clear

	echo '┌─┬─┬─┬─┐'
	echo '│ │a│b│c│'
	echo '├─┼─┼─┼─┤'
	echo '│1│ │ │ │'
	echo '├─┼─┼─┼─┤'
	echo '│2│ │ │ │'
	echo '├─┼─┼─┼─┤'
	echo '│3│ │ │ │'
	echo '└─┴─┴─┴─┘'
}

function setSymbol {
	error=false

	if [ ${#x} != 2 ]; then
		error=true
		return
	fi

	if [ $myTurn == true ]; then
		symbol=$mySymbol
	else
		symbol=$enemySymbol
	fi

	num=${x:1:1}

	case ${x:0:1} in
	'a')
		if [ "${a[$num]}" != " " ]; then
			error=true
			return
		fi

		a[$num]=$symbol
		col=3
		;;
	'b')
		if [ "${b[$num]}" != " " ]; then
			error=true
			return
		fi

		b[$num]=$symbol
		col=5
		;;
	'c')
		if [ "${c[$num]}" != " " ]; then
			error=true
			return
		fi

		c[$num]=$symbol
		col=7
		;;
	*)
		error=true
		return		
		;;
	esac

	row=$((3 + ($num - 1) * 2))

	tput sc
	tput cup $row $col
	echo -n $symbol
	tput rc
}

function checkGameOver {
	gameOver=false

	str=${a[1]}${a[2]}${a[3]}
	if [[ "$str" == "xxx" || "$str" == "ooo" ]]; then
		gameOver=true
		row[0]=3
		row[1]=5
		row[2]=7
		col[0]=3
		col[1]=3
		col[2]=3
		symbol=${a[1]}
		return
	fi

	str=${b[1]}${b[2]}${b[3]}
	if [[ "$str" == "xxx" || "$str" == "ooo" ]]; then
		gameOver=true
		row[0]=3
		row[1]=5
		row[2]=7
		col[0]=5
		col[1]=5
		col[2]=5
		symbol=${b[1]}
		return
	fi

	str=${c[1]}${c[2]}${c[3]}
	if [[ "$str" == "xxx" || "$str" == "ooo" ]]; then
		gameOver=true
		row[0]=3
		row[1]=5
		row[2]=7
		col[0]=7
		col[1]=7
		col[2]=7
		symbol=${c[1]}
		return
	fi

	str=${a[1]}${b[1]}${c[1]}
	if [[ "$str" == "xxx" || "$str" == "ooo" ]]; then
		gameOver=true
		row[0]=3
		row[1]=3
		row[2]=3
		col[0]=3
		col[1]=5
		col[2]=7
		symbol=${a[1]}
		return
	fi

	str=${a[2]}${b[2]}${c[2]}
	if [[ "$str" == "xxx" || "$str" == "ooo" ]]; then
		gameOver=true
		row[0]=5
		row[1]=5
		row[2]=5
		col[0]=3
		col[1]=5
		col[2]=7
		symbol=${a[2]}
		return
	fi

	str=${a[3]}${b[3]}${c[3]}
	if [[ "$str" == "xxx" || "$str" == "ooo" ]]; then
		gameOver=true
		row[0]=7
		row[1]=7
		row[2]=7
		col[0]=3
		col[1]=5
		col[2]=7
		symbol=${a[3]}
		return
	fi

	str=${a[1]}${b[2]}${c[3]}
	if [[ "$str" == "xxx" || "$str" == "ooo" ]]; then
		gameOver=true
		row[0]=3
		row[1]=5
		row[2]=7
		col[0]=3
		col[1]=5
		col[2]=7
		symbol=${a[1]}
		return
	fi

	str=${a[3]}${b[2]}${c[1]}
	if [[ "$str" == "xxx" || "$str" == "ooo" ]]; then
		gameOver=true
		row[0]=3
		row[1]=5
		row[2]=7
		col[0]=7
		col[1]=5
		col[2]=3
		symbol=${a[3]}
		return
	fi
}

function showWinCombination {
	tput sc
	tput setf 4

	for i in {0..2};
	do
		tput cup ${row[$i]} ${col[$i]}
		echo -n $symbol
	done

	tput rc
	tput setf 0
}

#start:

pipe1=/tmp/xo-pipe1
pipe2=/tmp/xo-pipe2

if [ "$1" == "-s" ]; then
	server=true
	mySymbol='x'
	enemySymbol='o'
	output=$pipe1
	input=$pipe2
	

	if [ ! -p $pipe1 ]; then
		mknod $pipe1 p
	fi

	if [ ! -p $pipe2 ]; then
		mknod $pipe2 p
	fi
else
	server=false
	mySymbol='o'
	enemySymbol='x'
	output=$pipe2
	input=$pipe1

	if [[ ! -p $pipe1 || ! -p $pipe2 ]]; then
		echo 'server is not running'
		exit 1
	fi
fi

gameRunning=true
myTurn=$server

initGame

while $gameRunning
do
	if [[ ! -p $pipe1 || ! -p $pipe2 ]]; then
		echo 'pipe was deleted'
		exit 1
	fi

	if [ $myTurn == true ]; then
		tput sc
		read -s -p 'your turn: ' x
		tput rc
		#прячем your turn
		echo -n '           '
		tput rc

		if [ "$x" == "exit" ]; then
			echo "you exited"
			gameRunning=false
			echo -n $x > $output
			break
		fi
	
		setSymbol
		if [ $error == true ]; then
			continue
		fi

		echo -n $x > $output
		myTurn=false

		checkGameOver
		if [ $gameOver == true ]; then
			echo "you win"
			gameRunning=false
		fi

	else
		read x<$input

		if [ "$x" == "exit" ]; then
			echo "your enemy exited"
			gameRunning=false
			break
		fi

		setSymbol
		if [ $error == true ]; then
			continue
		fi

		myTurn=true

		checkGameOver
		if [ $gameOver == true ]; then
			echo "you loose"
			gameRunning=false
		fi
	fi
done

showWinCombination

read x

clear

if [ $server == true ]; then
	if [ -p $pipe1 ]; then
		rm $pipe1
	fi

	if [ -p $pipe2 ]; then
		rm $pipe2
	fi
fi
