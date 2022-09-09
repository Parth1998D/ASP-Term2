currentDirectory=$PWD
tarDir=$PWD             # default tarDir if not provided
orgDir="$PWD/orgDir"    # default orgDir if not provided

usage() { echo "Usage: $0 [-o orgDir] [-t tarDir] <extension-list>" 1>&2; exit 1; }

while getopts ":t:o:" options; do                                              
  case "${options}" in                   
    t)                                
        tarDir=${OPTARG}

        # if provided tarDir not available
        if [ ! -d $tarDir ]; then
            echo "$tarDir: No such tarDir exists"
            exit 1;
        fi         
        ;;
    o)                                 
        orgDir=${OPTARG}                    
        ;;
    *)
        usage
    ;;
  esac
done

shift $(($OPTIND - 1))
extensionList="$@"

# create orgDir if does not exist
if [ ! -d $orgDir ]; then
    mkdir -p $orgDir
fi

# check for write permission in orgDir
if [ -w $orgDir ]; 
then
    for extension in $extensionList
    do
        fileList=`find $tarDir -type f -name "*.$extension"`

        IFS=$'\n'      # Change IFS to newline char

        for file in $fileList
        do
            fileName="$(basename "$file")"    # File name with Extension
            destinationfileName=$fileName     # Destination File name
            NAME="${fileName%.*}"             # File Name without extension

            cd $orgDir
            i=0
            while [ -f $destinationfileName ]
            do
                ((i++))
                destinationfileName="${NAME}_$i.$extension"
            done

            cd $currentDirectory
            cp $file $orgDir/$destinationfileName   # Copy files to orgDir
        done
        cd $orgDir

        # tar and remove only if files available
        fileCount=`ls -1 *.$extension 2>/dev/null | wc -l`
        if [ $fileCount != 0 ]; then
            tar -cf $extension.tar *.$extension
            rm *.$extension 
        fi

        cd $currentDirectory
    done
else
    echo "you don't have permission to write in $orgDir"
fi