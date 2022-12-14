/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package pkg374_instruction_encoder;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.HashMap;
import java.util.Arrays;
/**
 *
 * @author omnic
 */
public class InstructionEncoder {

    private static enum InstructionType {
        THREE_REGS,
        TWO_REGS,
        ONE_REGS,
        TWO_REGS_AND_IMMEDIATE,
        ONE_REGS_AND_IMMEDIATE,
        NO_OPS,
        LD_SPECIAL_CASE,
        ST_SPECIAL_CASE,
        UNKNOWN_TYPE
    }

    private HashMap<InstructionType, String> instRegexFormats;
    private HashMap<InstructionType, String[]> instructionGroups;
    private HashMap<String, String> instrOpcodes;
    private HashMap<String, String> branchInstrCodes;

    public void initInfo(){
        initInstrRegexes();
        initInstructionGroups();
        initInstrCodes();
    }

    private String getGroupStr(InstructionType g){
        HashMap<InstructionType, String> groupStrings = new HashMap<InstructionType, String>();
        groupStrings.put(InstructionType.THREE_REGS, "THREE_REGS");
        groupStrings.put(InstructionType.TWO_REGS, "TWO_REGS");
        groupStrings.put(InstructionType.ONE_REGS, "ONE_REGS");
        groupStrings.put(InstructionType.TWO_REGS_AND_IMMEDIATE, "TWO_REGS_AND_IMMEDIATE");
        groupStrings.put(InstructionType.ONE_REGS_AND_IMMEDIATE, "ONE_REGS_AND_IMMEDIATE");
        groupStrings.put(InstructionType.NO_OPS, "NO_OPS");
        groupStrings.put(InstructionType.LD_SPECIAL_CASE, "LD_SPECIAL_CASE");
        groupStrings.put(InstructionType.ST_SPECIAL_CASE, "ST_SPECIAL_CASE");
        groupStrings.put(InstructionType.UNKNOWN_TYPE, "UNKNOWN_TYPE");
        return groupStrings.get(g);
    }


    private void initInstrRegexes(){
        instRegexFormats = new HashMap<InstructionType, String>();
        instRegexFormats.put(InstructionType.THREE_REGS, "<inst> *<reg> *, *<reg> *, *<reg>");
        instRegexFormats.put(InstructionType.TWO_REGS, "<inst> *<reg> *, *<reg>");
        instRegexFormats.put(InstructionType.ONE_REGS, "<inst> *<reg>");
        instRegexFormats.put(InstructionType.TWO_REGS_AND_IMMEDIATE, "<inst> *<reg> *, *<reg> *, *<imm>");
        instRegexFormats.put(InstructionType.ONE_REGS_AND_IMMEDIATE, "<inst> *<reg> *, *<imm>");
        instRegexFormats.put(InstructionType.NO_OPS, "<inst> *");
        instRegexFormats.put(InstructionType.LD_SPECIAL_CASE, "<inst> *<reg> *, *<imm> *(\\(<reg>\\))*");
        instRegexFormats.put(InstructionType.ST_SPECIAL_CASE, "(st) *<imm> *(\\(<reg>\\))* *, *<reg>");

        HashMap<String, String> regexReplacements = new HashMap<String, String>();
        regexReplacements.put("<inst>", "(inst)");
        regexReplacements.put("<reg>", "([rR][0-9]*)");
        regexReplacements.put("<imm>", "(\\$[0-9]*[a-f]*[A-F]*[0-9]*|-*[0-9]*)");


        for ( InstructionType k : instRegexFormats.keySet()) {
            String regvalue = instRegexFormats.get(k);
            for ( String symbol : regexReplacements.keySet() ) {
                regvalue = regvalue.replace(symbol, regexReplacements.get(symbol));
            }
            instRegexFormats.put(k, regvalue);
        }
    }

    private void initInstructionGroups(){
        instructionGroups = new HashMap<InstructionType, String[]>();
        instructionGroups.put(InstructionType.THREE_REGS, new String[]{"add","sub","shr","shl","ror","rol","and","or"});
        instructionGroups.put(InstructionType.TWO_REGS, new String[]{"mul", "div", "neg", "not"});
        instructionGroups.put(InstructionType.ONE_REGS, new String[]{"jr", "jal", "in", "out", "mfhi", "mflo"});
        instructionGroups.put(InstructionType.TWO_REGS_AND_IMMEDIATE, new String[]{"andi", "addi", "ori"});
        instructionGroups.put(InstructionType.ONE_REGS_AND_IMMEDIATE, new String[]{"brzr", "brnz", "brpl", "brmi"});
        instructionGroups.put(InstructionType.NO_OPS, new String[]{"nop", "halt"});
        instructionGroups.put(InstructionType.LD_SPECIAL_CASE, new String[]{"ld", "ldi"});
        instructionGroups.put(InstructionType.ST_SPECIAL_CASE, new String[]{"st"});
    }

    private void initInstrCodes(){
        instrOpcodes = new HashMap<String, String>();
        instrOpcodes.put("add","00011");
        instrOpcodes.put("sub","00100");
        instrOpcodes.put("shr","00101");
        instrOpcodes.put("shl","00110");
        instrOpcodes.put("ror","00111");
        instrOpcodes.put("rol","01000");
        instrOpcodes.put("and","01001");
        instrOpcodes.put("or","01010");
        instrOpcodes.put("addi","01011");
        instrOpcodes.put("andi","01100");
        instrOpcodes.put("ori","01101");
        instrOpcodes.put("mul","01110");
        instrOpcodes.put("div","01111");
        instrOpcodes.put("neg","10000");
        instrOpcodes.put("not","10001");
        instrOpcodes.put("jr","10011");
        instrOpcodes.put("jal","10100");
        instrOpcodes.put("in","10101");
        instrOpcodes.put("out","10110");
        instrOpcodes.put("mfhi","10111");
        instrOpcodes.put("mflo","11000");
        instrOpcodes.put("nop","11001");
        instrOpcodes.put("halt","11010");
        instrOpcodes.put("ld","00000");
        instrOpcodes.put("ldi","00001");
        instrOpcodes.put("st","00010");
        instrOpcodes.put("brzr","10010");
        instrOpcodes.put("brnz","10010");
        instrOpcodes.put("brpl","10010");
        instrOpcodes.put("brmi","10010");

        branchInstrCodes = new HashMap<String, String>();
        branchInstrCodes.put("brzr","0000");
        branchInstrCodes.put("brnz","0001");
        branchInstrCodes.put("brpl","0010");
        branchInstrCodes.put("brmi","0011");
    }

    private static boolean isElementInArray(String elem, String[] arr){
      return Arrays.asList(arr).contains(elem);
    }

    private InstructionType getInstrType(String instName){
        //returns the instruction type if there is any, if not returns unknown 
        for (InstructionType t : instructionGroups.keySet()){
            if ( isElementInArray(instName, instructionGroups.get(t)) ) return t;
        }
        return InstructionType.UNKNOWN_TYPE;
    }

    public String getInstrFormat(String instName){
        InstructionType type = getInstrType(instName);
        String res = instName + " ";
        switch(type){
            case THREE_REGS:
                res += "Ra, Rb, Rc";
                break;
            case TWO_REGS:
                res += "Ra, Rb";
                break;
            case ONE_REGS:
                res += "Ra";
                break;
            case NO_OPS:
                res = instName;
                break;
            case TWO_REGS_AND_IMMEDIATE:
                res += "Ra, Rb, C";
                break;
            case ONE_REGS_AND_IMMEDIATE:
                res += "Ra, C";
                break;
            case LD_SPECIAL_CASE:
                res = "'" + instName + " Ra, C' or '" + instName + " Ra, C(Rb)'";
                break;
            case ST_SPECIAL_CASE:
                res = "'st C, Ra' or 'st C(Rb), Ra'";
                break;
            default:
                res = "Unknown Instruction";
                break;
        }
        return res;
    }

    private String makePartsStr(String[] instrParts){
        String a_str = "Instruction: (" + instrParts[0] + ")" + "\n";
        a_str += "Ra: ("+instrParts[1] + ")" + "\n";
        a_str += "Rb: ("+instrParts[2] + ")" + "\n";
        a_str += "Rc: ("+instrParts[3] + ")" + "\n";
        a_str += "Cout: ("+instrParts[4] + ")";

        a_str = "[ ("+instrParts[0]+"), ("+instrParts[1]+"), ("+instrParts[2]+"), ("+instrParts[3]+"), ("+instrParts[4]+") ]";
        return a_str;
    }

    private void cannotEncodeInstruction(String[] instrParts, InstException.ErrorType et) throws InstException{
        throw new InstException("Could Not Encode: "+makePartsStr(instrParts), et);
    }


    public static String decimalToBinary(Long number, int length){
        String padding_bit = (number >= 0) ? "0" : "1";

        //get binary value for number
        //if number is negative, it is padded with excess 1s
        String binStr = Long.toBinaryString(number);

        if (length != 0 && binStr.length() != length){
            //need to add or remove characters
            if ( binStr.length() < length){
                //pad it if it is less than required length
                int org_length = binStr.length();
                for(int i=1; i<= length-org_length; i++) binStr = padding_bit + binStr;
            } else {
                //need to remove bits
                binStr = binStr.substring(binStr.length() - length);
            }
        }

        return binStr;
    }

    public static String decimalToHex(Long number, int length){
        String padding_bit = (number >= 0) ? "0" : "f";

        //get hex for number
        //if number is negative, it is padded with extra fs
        String hexStr = Long.toHexString(number);

        if (length != 0 && hexStr.length() != length){
            //need to add or remove characters
            if ( hexStr.length() < length){
                //pad it if it is less than required length
                int org_length = hexStr.length();
                for(int i=1; i<= length-org_length; i++) hexStr = padding_bit + hexStr;
            } else {
                //need to remove bits
                hexStr = hexStr.substring(hexStr.length() - length);
            }
        }

        return hexStr;
    }

    private String makeInstructionFromParts(String[] instrParts, int outputBase) throws InstException{
        //takes the input instruction parts and makes the output in the specified base
        String full_encoded_instr = "";
        int bits_not_set = 32;

        String instrName = instrParts[0];
        String instrCout = instrParts[4];

        //get the opcode for the instruction
        String opcode = instrOpcodes.get(instrName);
        if ( opcode == null) cannotEncodeInstruction(instrParts, InstException.ErrorType.PARSING_UNKOWN_INSTRUCTION);

        //add it to the instruction
        full_encoded_instr += opcode;
        bits_not_set -= 5;

        //encode the registers if present
        for (int i=1; i<=3; i++){
            if (!instrParts[i].equals("")){
                full_encoded_instr += decimalToBinary( Long.parseLong(instrParts[i]), 4);
                bits_not_set -= 4;
            }
        }

        //branch has special case
        if(instrName.equals("brzr") || instrName.equals("brnz") || instrName.equals("brpl") || instrName.equals("brmi")){
            full_encoded_instr += branchInstrCodes.get(instrName);
            bits_not_set -= 4;
        }

        //now handle cout if present
        if ( !instrCout.equals("")){
            //convert the cout to binary
            Long coutDecimal;
            if(instrCout.charAt(0) == '$'){
                //the value is hex, so we convert it from hex first and then get binary representation
                coutDecimal = Long.parseLong(instrCout.substring(1), 16);
            } else {
                coutDecimal = Long.parseLong(instrCout);
            }
            //it will then take up the remaining space
            String coutInInstr = decimalToBinary(coutDecimal, bits_not_set);

            full_encoded_instr += coutInInstr;
            bits_not_set -= coutInInstr.length();
        } else{
            //no cout
            //pad with 0s
            String padding = "";
            for(int i=0; i<bits_not_set; i++){
                padding = "0" + padding;
            }
            full_encoded_instr += padding;
            bits_not_set -= padding.length();
        }

        if (full_encoded_instr.length() != 32) cannotEncodeInstruction(instrParts, InstException.ErrorType.ENCODING_BAD_BIT_COUNT);

        if (outputBase != 2){
            if (outputBase == 16){
                full_encoded_instr = decimalToHex( Long.parseLong(full_encoded_instr,2 ), 32/4).toUpperCase();
            } else throw new InstException(InstException.ErrorType.BAD_OUTPUT_BASE);
        }

        return full_encoded_instr;
    }

    private void prepPartsForEncoding(String[] instrParts){
        //takes the parsed instruction string parts and applies any special instruction requirements
        //InstrParts = [ inst, ra, rb, rc, c_out ]
        //also strips registers to just raw values

        //special behaviour for jal instruction:
        if ( instrParts[0].equals("jal") ){
            //set rb and cout
            instrParts[2] = "r15";
            instrParts[4] = "1";
        }

        //special behaviour for load and store
        InstructionType type = getInstrType(instrParts[0]); 
        if (  type == InstructionType.LD_SPECIAL_CASE || type == InstructionType.ST_SPECIAL_CASE  ){
            //if there is no rb, the default is adding to r0 for load and store
            if ( instrParts[2].equals("") ) instrParts[2] = "r0";
        }

        //strip registers of the r
        for (int i=1; i <= 3; i++){
            if(!instrParts[i].equals("")) instrParts[i] = instrParts[i].substring(1);
        }
    }

    private static String[] getMatchedGroups(String searchstr, String regex, InstructionType type){
        Pattern r = Pattern.compile(regex);
        Matcher m = r.matcher(searchstr);

        if (!m.find()) return new String[] {""};

        //only the special cases are allowed to not match the entire string
        if ( !m.group(0).equals(searchstr)){
            if ( type != InstructionType.LD_SPECIAL_CASE || type != InstructionType.ST_SPECIAL_CASE ) return new String[] {""};
        }

        //create the new string array that just has the matched parts
        String[] groups = new String[m.groupCount()];

        for (int i=1; i <= m.groupCount(); i++){
            groups[i-1] = (m.group(i) == null) ? "" : m.group(i);
        }

        return groups;

    }

    private void cannotParseInstruction(String instr, InstException.ErrorType et) throws InstException{
        if (instr.equals("")){
            throw new InstException(et);
        } else {
            throw new InstException("Could Not Parse Instruction: '" + instr + "'", et);
        }
    }

    private String[] parseInstruction(String instructionStr) throws InstException{
        //parses the 
        instructionStr = instructionStr.strip();

        if ( instructionStr.equals("") ) throw new InstException(InstException.ErrorType.EMPTY_INSTRUCTION);

        String instName = instructionStr.split(" ")[0];

        //get the group for the instruction to get the parsing regex
        InstructionType curType = getInstrType(instName);

        if ( curType == InstructionType.UNKNOWN_TYPE ) cannotParseInstruction(instName, InstException.ErrorType.PARSING_UNKOWN_INSTRUCTION);

        String parseRegex = instRegexFormats.get(curType).replace("inst", instName);

        String[] groups = getMatchedGroups(instructionStr, parseRegex, curType);


        if (groups.length == 0) cannotParseInstruction(instructionStr, InstException.ErrorType.PARSING_BAD_FORMAT);

        //if we can parse it then handle the args based on the current type
        //values for          RA,  RB, RC
        String[] regArray = { "" , "", "" };
        String Cout = "";

        //make them all lowercase
        for (int i=0; i<groups.length; i++){
            groups[i] = groups[i].toLowerCase();
        }

        //check if we got the instruction right
        if ( !groups[0].equals(instName) )  cannotParseInstruction(instructionStr, InstException.ErrorType.PARSING_BAD_FORMAT);

        //only registers case
        if ( curType == InstructionType.THREE_REGS || curType == InstructionType.TWO_REGS || curType == InstructionType.ONE_REGS || curType == InstructionType.NO_OPS){
            //in this groups would be [ inst, ra, rb, rc]
            //where ra, rb, rc are dependent on the type

            //use the register count (group count -1) to determine if the instruction is valid for the type
            //that is, if it has the required number of registers
            switch(groups.length-1){
                case 0:
                    if ( curType != InstructionType.NO_OPS) cannotParseInstruction(instructionStr, InstException.ErrorType.PARSING_INCORRECT_ARGUMENT_COUNT);
                    break;
                case 1:
                    if ( curType != InstructionType.ONE_REGS) cannotParseInstruction(instructionStr, InstException.ErrorType.PARSING_INCORRECT_ARGUMENT_COUNT);
                    break;
                case 2:
                    if ( curType != InstructionType.TWO_REGS) cannotParseInstruction(instructionStr, InstException.ErrorType.PARSING_INCORRECT_ARGUMENT_COUNT);
                    break;
                case 3:
                    if ( curType != InstructionType.THREE_REGS) cannotParseInstruction(instructionStr, InstException.ErrorType.PARSING_INCORRECT_ARGUMENT_COUNT);
                    break;
                default:
                    //any other value is not allowed
                    cannotParseInstruction(instructionStr, InstException.ErrorType.PARSING_INCORRECT_ARGUMENT_COUNT);
                    break;
            }

            //go through the groups and populate register
            for (int i=1; i < groups.length; i++){
                regArray[i-1] = groups[i];
            }
        }

        //immediate value
        if ( curType == InstructionType.TWO_REGS_AND_IMMEDIATE || curType == InstructionType.ONE_REGS_AND_IMMEDIATE ){
            //group is [ inst, ra, cout] or [inst ra, rb, cout]
            // where rb is dependent on type

            //check the group arguments again (sans instruction)
            switch(groups.length-1){
                case 2:
                    if ( curType != InstructionType.ONE_REGS_AND_IMMEDIATE) cannotParseInstruction(instructionStr, InstException.ErrorType.PARSING_INCORRECT_ARGUMENT_COUNT);
                    break;
                case 3:
                    if ( curType != InstructionType.TWO_REGS_AND_IMMEDIATE) cannotParseInstruction(instructionStr, InstException.ErrorType.PARSING_INCORRECT_ARGUMENT_COUNT);
                    break;
                default:
                    //any other value is not allowed
                    cannotParseInstruction(instructionStr, InstException.ErrorType.PARSING_INCORRECT_ARGUMENT_COUNT);
                    break;
            }

            //get our registers
            // groups sans inst, sans cout
            for(int i=1; i <= groups.length-2; i++){
                regArray[i-1] = groups[i];
            }

            //get our cout value, always the last one
            Cout = groups[ groups.length-1];
        }

        else if ( curType == InstructionType.LD_SPECIAL_CASE ){
            //ld Ra, C or ld Ra, C(Rb) (applies for ldi as well)
            //groups could be [inst, Ra, C, "", ""] and [inst, Ra, C, (Rb), Rb]

            //ra and c must always be present, raise exception if they are not
            if ( groups[1].equals("") ||  groups[2].equals("") ) cannotParseInstruction(instructionStr, InstException.ErrorType.PARSING_INCORRECT_ARGUMENT_COUNT);

            //get our always present values
            regArray[0] = groups[1];
            Cout = groups[2];

            //if we have the rb argument take it
            if ( !groups[3].equals("") && groups[3].equals("("+groups[4]+")")) regArray[1] = groups[4];

        } 

        else if ( curType == InstructionType.ST_SPECIAL_CASE ){
            //st C, Ra or st C(Rb), Ra
            // [ inst, c, "", "", ra] or [ inst, c, (rb), rb, ra]

            //ra and c must always be present, raise exception if they are not
            if ( groups[1].equals("") ||  groups[4].equals("") ) cannotParseInstruction(instructionStr, InstException.ErrorType.PARSING_INCORRECT_ARGUMENT_COUNT);

            //get our always present values
            regArray[0] = groups[4];
            Cout = groups[1];

            //if we have the rb argument take it
            if ( !groups[2].equals("") && groups[2].equals("("+groups[3]+")")) regArray[1] = groups[3];
        }

        return new String[]{ instName, regArray[0], regArray[1], regArray[2], Cout };
    }

    private String _encodeInstruction(String instructionStr, int outputBase) throws InstException{
        //parse the instruction
        //returns array in format:
        //[ instruction name, ra, rb, rc, c_out]
        //if the registers do not exist then they will be empty string
        String[] instrParts = parseInstruction(instructionStr);
        prepPartsForEncoding(instrParts);
        return makeInstructionFromParts(instrParts, outputBase);
    }

    public String encodeInstruction(String instructionStr, int outputBase) throws InstException{
        //encodes the given instruction string to an ouput base
        return _encodeInstruction(instructionStr, outputBase);
    }

    public String encodeInstruction(String instructionStr) throws InstException{
        //encodes the given instruction string to an ouput base
        return _encodeInstruction(instructionStr, 16);
    }

    public InstructionEncoder(){
        initInfo();
    }


}
