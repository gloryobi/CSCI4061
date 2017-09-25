#!/bin/bash

#---------------------------------------------------------------------
# CSci 4061 Fall 2017
# Assignment# 1
# This work must be done individually.
# Student name: Glory Obielodan
# Student id: 4964466
# x500 id: obiel001
# Operating system on which you tested your code: Linux
# CSELABS machine: <CSEL-KH1250-03>
#---------------------------------------------------------------------


echo " "

######################
# Directives for usage

 if [[ $argv == 0 ]]
   then
    output="Process argument missing \n" 
    echo -e $output
   else
    output="SELECT THE FUNCTION YOU WANT TO EXECUTE: \n" 
    echo -e $output
    output="1. Search for files" 
    echo -e $output
    output="2. Calculate total length" 
    echo -e $output
    output="3. Find zero length files" 
    echo -e $output
    output="4. Create backup files" 
    echo -e $output
    output="5. Exit" 
    echo -e $output
       
 fi
  
  echo -e -n "\nEnter option: "
  read option
  
  case "$option" in

	1)  echo -e "\nSearchng for files"
	    #here you should implement for search files
	    # Begin Code
	    
	    echo -n "Enter the directory name: "
            read dir					#get directory name from user

            if [ ! -d $dir ]				#check to see if directory does not exist
            then
            	echo "$dir does not exist"
      	    else
		echo -n "Enter the file name: "		
        	read file				#get file name from user
		
		echo "-------------------------------------------------------------------------"

        	list="$(find "$dir" -name "$file")"	#get a list of files in the directory that are named $file

        	for i in $list				#parse through list
        	do
			echo

			readlink -f $i			#command to print out fullpath name
          		if [ -d $i ]			#if $i is a directory
          		then
            			ls -ld $i
          		else
            			ls -l $i
          		fi
        	done
		echo "-------------------------------------------------------------------------"
      	    fi
		
	    #End Code
	    ;;
	2)  echo -e "\nCalculating total of the size of all files in the directory tree"
	    #here you should implement the code to calculate the size of all files in a folder
	    # Begin Code
	    
	    echo -n "Enter the directory name: "
            read dir					#get directory name from user

            if [ ! -d $dir ]				#check to see if directory does not exist
            then
            	echo "$dir does not exist"
	    else
		total=0
		filelist="$(find "$dir")"		#get a list of all files in directory

		for i in $filelist
		do
			size=`stat -c%s $i`		#get the size of file $i
			total=`expr $total + $size`	#add $size to $total
		done

		echo "-------------------------------------------------------------------------"
		echo "The sum of the size of all files in the directory \"$dir\" is: $total"
		echo "-------------------------------------------------------------------------"
	    fi

	    #End Code
	    ;;
	    
	3) echo -e "\nFinding zero length files"
	    #here you should implement the code to find empty files
	    # Begin Code
	    
	    echo -n "Enter the directory name: "
            read dir					#get the directory name from user

            if [ ! -d $dir ]				#check to see if directory does not exist
            then
            	echo "$dir does not exist"
	    else
		list=`find $dir -maxdepth 1 -size 0b`	#get list of file size 0 limited to current file only (maxdepth=1)
		
		echo "-------------------------------------------------------------------------"

		for i in $list
		do
			readlink -f $i			#print out fullpath name of $i
		done
		
		echo "-------------------------------------------------------------------------"
	    fi

	    #End Code
	    ;;
	    
	 4) echo -e "\nCreating backup files"
	    #here you should implement the backup code  
	    # Begin Code
	    
	    echo -n "Enter the directory name: "
            read dir					#get the directory name from user

            if [ ! -d $dir ]				#check to see if directory exists
            then
            	echo "$dir does not exist"
	    else
		echo -n "Enter the file name pattern: "
        	read file				#get the file name from user

      		date=`date +%m-%d-%Y`			#$date contains date string
      		echo

      		filelist=`find $dir`			#get a list of all files in the directory

      		echo "-------------------------------------------------------------------------"

      		for i in $filelist
      		do
        		if [ -f $i ]			#check to see if $i is an ordinary file
       			then
				if [[ $i == *.bak ]]	#check to see if $i is backup file
				then
					: #do nothing
				else
          				backup=$i.bak
          				if [ -e $backup ]	#checking if a backup file already exists
          				then
            					echo "A backup file already exists for file \"$i\"."

            					if diff $i $backup			#comparing $i with it's backup file
            					then
              						echo "The backup file for file \"$i\" is up to date."		#backup file is the same as $i
            					else
              						echo "The backup file for file  \"$i\" is not up to date."	#backup file is different from $i
              						echo "Moving the old backup file to $backup-$date."
             	 					mv $backup $backup-$date
              						echo "Creating a new backup file for file \"$i\"."
              						cp $i $backup							#create a new backup file
            					fi
          				else
            					echo "No backup file exists for the file \"$i\"."
						echo "Creating backup file \"$i.bak\"."
            					cp $i $backup
          				fi
					echo
				fi
        		fi
      		done

		echo "-------------------------------------------------------------------------"
      		echo
		
		filelist=`find "$dir" -name "$file"`
		
		for i in $filelist
		do
      			if [ -d $i ]			#make sure $i is a directory
      			then
				if [[ $i == *.bak ]]	#check to see if $i is backup directory
				then
					: #do nothing
				else
          				backupDir=$i.bak
          				if [ -e $backupDir ]		#checking is a backup directory already exists
          				then
            					echo "The directory \"$i\" already has a backup."			#make a new one if it does
            					echo "Moving the old backup directory to $backupDir-$date."		#and move the old one to a $date extension
            					mv $backupDir $backupDir-$date
           					echo "Creating a new backup directory for directory \"$i\"."
            					cp -r $i $backupDir
          				else
            					echo "No backup directory exists for $i."				#create a backup file if one does not exist
						echo "Creating backup directory \"$backupDir\"."
            					cp -r $i $backupDir
          				fi
					echo
				fi
        		fi
		done

		echo "-------------------------------------------------------------------------"
      	    fi

	    #End Code
	    ;;
	    
	5) echo "Exiting the program. Thanks"
	  exit
	  ;;
   esac
