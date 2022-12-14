/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package pkg374_instruction_encoder;

/**
 *
 * @author omnic
 */
public class InstException extends Exception{
    ErrorType et;
    String ourmsg;

    public static enum ErrorType{
        NOT_SPECIFIED,
        UNKNOWN_ERROR,
        EMPTY_INSTRUCTION,
        PARSING_UNKOWN_INSTRUCTION,
        PARSING_BAD_FORMAT,
        PARSING_NO_PARSED_GROUPS,
        PARSING_DIFFERENT_INSTRUCTION,
        PARSING_INCORRECT_ARGUMENT_COUNT,
        ENCODING_BAD_BIT_COUNT,
        BAD_OUTPUT_BASE
    }

    public InstException() {
        ourmsg = "";
        et = ErrorType.NOT_SPECIFIED;
    }

    public InstException(String msg) {
        ourmsg = msg;
        et = ErrorType.NOT_SPECIFIED;
    }

    public InstException(ErrorType etin) {
        ourmsg = "";
        et = etin;
    }


    public InstException(String msg, ErrorType etIn) {
        ourmsg = msg;
        et = etIn;
    }

    private static String getErrorTypeStr(ErrorType an_et){
        String str_out = "";
        switch(an_et){
            case NOT_SPECIFIED:
                break;
            case UNKNOWN_ERROR:
                str_out = "Unknown Error";
                break;
            case EMPTY_INSTRUCTION:
                str_out = "Empty Instruction";
                break;
            case PARSING_UNKOWN_INSTRUCTION:
                str_out = "Unknown Instruction";
                break;
            case PARSING_NO_PARSED_GROUPS:
                str_out = "No Arguments could be parsed";
                break;
            case PARSING_DIFFERENT_INSTRUCTION:
                str_out = "Instruction Parsed is different from expected instruction";
                break;
            case PARSING_INCORRECT_ARGUMENT_COUNT:
                str_out = "Incorrect Number of Arguments Parsed";
                break;
            case PARSING_BAD_FORMAT:
                str_out = "Incorrect Format for Instruction";
                break;
            case ENCODING_BAD_BIT_COUNT:
                str_out = "Encoded instruction was not 32 bits";
                break;
            case BAD_OUTPUT_BASE:
                str_out = "Bad output base for instruction, only binary and hex are supported";
                break;
            default:
                break;
        }
        return str_out;
    }

    public ErrorType getErrorType(){
        return et;
    }

    public String getMessageOnly(){
        return ourmsg;
    }

    public String getMessage(){
        String out = ( !ourmsg.equals("") ) ? ourmsg : "";
        //add the error message
        out = ( out.equals("") ) ? getErrorTypeStr(et) : out + ((!getErrorTypeStr(et).equals("")) ? ", "+getErrorTypeStr(et) : "");
        return out;
    }
}
