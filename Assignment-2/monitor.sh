function descendentChecker(){
    pids=$(ps -u | awk '{print $2}' | grep -o '[0-9]\+')
    pidsArray=( $pids )
    pidArrayLen=${#pidsArray[@]}
    
    myMail=""
    
    for (( i=0; i<$pidArrayLen; i++ ))
    do  
        pid=${pidsArray[$i]}
        timestamp=`date`
        
        pstree=`pstree -T -p $pid | grep -o '([0-9]\+)' | grep -o '[0-9]\+' | sort -n`
        pstreeArray=( $pstree )
        
        descendentCount=${#pstreeArray[@]}
        let descendentCount--
        
        if [ $descendentCount -gt 6 ]
        then
            echo ""
            echo "time:$timestamp"
            echo -e "pid:$pid     descendents:$descendentCount"
            echo -e "ProcID\tPPid\tLevel"
            
            myMail+="\ntime:$timestamp\n"
            myMail+="pid:$pid\tdescendents:$descendentCount\n"
            myMail+="ProcID  PPid\tLevel\n"
            
            for (( j=1; j<=$descendentCount; j++ ))
            do
                descendant=${pstreeArray[$j]}
                ppid=`ps -o ppid= -p $descendant`
                
                if [ -z "$ppid" ]
                then
                    echo -e "$descendant\tNA\tNA"
                    myMail+="$descendant\tNA\tNA\n"
                else
                    level=0
                    z=$descendant
                    
                    while [ $z -ne $pid ]
                    do
                    z=`ps -o ppid= -p $z`
                    let level++
                    done
                    
                    echo -e "$descendant\t$ppid\t$level"
                    myMail+="$descendant\t$ppid\t$level\n"
                fi
            done
        fi
    done
    echo -e $myMail | mail -s "More than 6 descendents detected"  parthmd456@gmail.com
    echo -e $myMail >> log.txt
}

function userProcessLogger(){
    while(true)
    do
        ps -u >> log.txt
        sleep 300
    done
}

userProcessLogger &

function checkCaller(){
    while(true)
    do
        descendentChecker
        sleep 15
    done
}

checkCaller