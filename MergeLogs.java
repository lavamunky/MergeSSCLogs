import org.apache.commons.cli.*;
import java.io.PrintWriter;
import java.util.List;
import java.util.Properties;

public class MergeLogs
{
    
    


    public static void main(String[] args)
    {
        //Option separateLog = new Option("f", "output", true, "Prints the merged instance to a separate log file");
        //creating options
        
        Properties properties = new Properties();
        properties.setProperty("OUT", "output");
        properties.setProperty("COMBINE", "combine");
        properties.setProperty("HELP", "help");
        
        final Option separateLog = OptionBuilder.withArgName("filename")
                             .hasArg()
                             .isRequired(false)
                             .withDescription("Prints the merged instance to a separate log file, filename")
                             .withLongOpt("output")
                             .create("o");
        final Option combine = OptionBuilder.isRequired(false)
                        .withDescription("Prints the merged log into the first log given")
                        .withLongOpt("combine")
                        .create("s");
        final Option help = OptionBuilder.isRequired(false)
                            .withDescription("Prints this message")
                            .withLongOpt("help")
                            .create("h");

        //above arguments are mutually exclusive, so put in OptionGroup
        OptionGroup optgrp = new OptionGroup();
        optgrp.addOption(separateLog);
        optgrp.addOption(combine);
        
        Options options = new Options();
        options.addOptionGroup(optgrp);
        options.addOption(help);

        CommandLineParser parser = new PosixParser();
        CommandLine cmd = null;
        
        //System.out.println("debug 0");

        try {
            cmd = parser.parse(options, args);

        } catch (ParseException pe){
            System.out.println("Parsing command line failed. Reason: " + pe.getMessage());
            summary(options);
        } 
        //NOTE TO SELF: NEED TO CHECK FOR FILENAME ON separateLog

        //System.out.println("debug 1");

        //Debugging purposes
        Option[] commands = cmd.getOptions();
        List files = cmd.getArgList();

        //System.out.println("debug 2");

        if (files.size()!=2){
            System.out.println("Wrong number of arguments");
            summary(options);
        }

        //System.out.println("debug 3");
      
        for (Option opt: commands)
        {
            System.out.println("getArgName: " + opt.getArgName());
            System.out.println("getOpt: " + opt.getOpt());
            System.out.println("getValue: " + opt.getValue());
            if (opt.equals(separateLog)){
                System.out.println("equals separateLog");
            } else if (opt.equals(combine)){
                System.out.println("equals combine");
            }
        }
        if (commands.length == 0)
            System.out.println("stdout");

        

        //summary(options);

        /*
        String[] files = new String[3]; // [outputFile, firstMergeFile, secondMergeFile]
        //parse command line
        files = parseCommandLine(args);
        //open files

        for (String str : files)
            System.out.println(str);
        */

        //parse start of lines

        //merge by date & time

    }
    
    static void summary(Options opts)
    {
        //generate help statement
        HelpFormatter formatter = new HelpFormatter();
        formatter.printHelp("MergeLogs logfile_1 logfile_2", opts, true);
        System.exit(1);
    }
}

