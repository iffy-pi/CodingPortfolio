/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package pkg374_instruction_encoder;
import java.io.File;
import java.io.FileNotFoundException;
import java.util.Scanner;

/**
 *
 * @author omnic
 */
public class Main {

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        BulkInstFrame bf = new BulkInstFrame();
    }

    public static int run_encoder_tests(){
        //creating File instance to reference text file in Java
        File text = new File("C:\\Users\\omnic\\OneDrive\\Documents\\Coding Misc\\sample_instr.txt");
     
        //Creating Scanner instance to read File in Java
        Scanner scnr;
        try {
            scnr = new Scanner(text);
        } catch (FileNotFoundException e) {
            print("Could not find file!!!");
            return 1;
        }

        InstructionEncoder instrEncder = new InstructionEncoder();
     
        //Reading each line of instruction file to test instruction
        int lineNumber = 1;
        while(scnr.hasNextLine()){
            String line = scnr.nextLine();
            String output = "";
            try {
                output = instrEncder.encodeInstruction(line);
            } catch(Exception e){
                output = e.getMessage();
            }
            print("===========================");
            print("Input: "+line);
            print("Output:");
            print(output);
            print("===========================");
        }
        return 0;

    }

    public static void print(String in){
        System.out.println(in);
    }


    public static void test_encoder(String instr){
        InstructionEncoder instrEncder = new InstructionEncoder();
        try {
            String output = instrEncder.encodeInstruction(instr);
            print(output);
        } catch(InstException e){
            print(e.getMessage());
        }
    }
    //https://www.oracle.com/technical-resources/articles/javase/single-jar.html -- creating the jar file the right way
}
