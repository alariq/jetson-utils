#DST=$1
#DST=pi@nanopi:jetson-utils-test
#DST=cat@lubancat:jetson-utils-test
DST=cat@10.5.0.2:jetson-utils-test
echo "syncing to $DST"

# -c options might be useful if clocks are not the same
#
rsync -rzcP --exclude=.git\
    --exclude=.cache\
    --exclude=.ccls-cache\
    --exclude=.vscode\
    --exclude=build\
    --exclude=build-rknn\
    --exclude=build-nocuda\
    --exclude=dump\
    . $DST
