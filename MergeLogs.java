public class MergeLogs
{
    static int option;
    static String outFile;
    public static void main(String[] args)
    {
        String[] files = new String[3]; // [outputFile, firstMergeFile, secondMergeFile]
        //parse command line
        files = parseCommandLine(args);
        //open files

        for (String str : files)
            System.out.println(str);

        //parse start of lines

        //merge by date & time

    }

    static String[] parseCommandLine(String[] cli)
    {
        if (cli.length<2 || cli.length>4)
            summary();
        
        int index = parseOptions(cli);
        //validation of number of arguments given
        if (option==0) //no switches given, should be 2 args
        {
            if (cli.length!=2) {summary();};
        }
        else if (option==1) //-f logfile.log set, should be 4 args
        {
            if (cli.length!=4) {summary();};
        }
        else if (option==2) //-s set, should be 3 args
        {
            if (cli.length!=3) {summary();};
        }
        if (option==0)
            outFile=cli[index];
        else if (option==2)
            outFile=null;
        String[] files = {outFile, cli[index], cli[index+1]};
        return files;
    }

    static int parseOptions(String[] cli)
    {
        option=0;
        int index=0; //index of argument after any options
        for (int i=0; i<cli.length; i++)
        {
            if (cli[i].charAt(0) == '-') //a switch
            {
                if (option!=0) //already changed, therefore multiple switches. Not currently allowed
                    summary();
                if (cli[i].length()>2) //not particularly allowing for expandibility. But there are better ways to parse command line args
                {
                    System.out.println("Only one option allowed at a time");
                    summary();
                }
                switch (cli[i].charAt(1)){ //check if one of the known options. Very few, so quicker
                    case 'f':   if (cli.length>(i+1) && cli[i+1] != null)
                                {
                                    outFile=cli[i+1];
                                    option=1;
                                    index=i+2; //takes additional argument
                                }else
                                    summary();
                                break;
                    case 's':   option=2;
                                index=i+1; //takes no additional arguments
                                break;
                    default:    summary();
                                break;
                }
            }
        }
        return index;

    }
    static void summary()
    {
        System.out.println("\tThis software is intended for merging logs, specifically those for Fortify 360 Server/HP Fortify Software Security Center");
        System.out.println("\tIt looks at the date of each line and then combines the logs, either to stdout, the first log, or another log");
        System.out.println("\tMergeLogs [OPTIONS] log1.log log2.log\n\t-f logfile.log\tPrints the merged instance to a separate log file, in this case logfile.log\n\t-s\t\tPrints the merged log into the first log given\n\nIf no switches are given, the merged log is printed to stdout\n");
        System.exit(1);
    }
}

