RAMFS_OUTPUT=iso/root/ramfs.tar

FON_NAME=toshibatxl1_bmplus.sf2
RAMFS_FILES+=iso/$(FON_NAME)

# --transform part is to strip the directory
# of file names. You may have to rewrite this
# line if using non-GNU tar.
$(RAMFS_OUTPUT): $(RAMFS_FILES)
	tar --transform 's/.*\///g' --format=ustar -cf $@ $^

ramfs_clean:
	rm -f $(RAMFS_OUTPUT)

ramfs: $(RAMFS_OUTPUT)
