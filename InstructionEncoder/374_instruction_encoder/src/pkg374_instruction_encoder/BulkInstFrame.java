/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package pkg374_instruction_encoder;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.HashMap;
import static javax.swing.JOptionPane.showMessageDialog;

/**
 *
 * @author omnic
 */
public class BulkInstFrame extends javax.swing.JFrame {

    /**
     * Creates new form InstFrame_1
     * 
     * 
     */
    InstructionEncoder instrEncoder;
    public BulkInstFrame() {
        initComponents();
        setLocationRelativeTo(null);
        setVisible(true);
        instrEncoder = new InstructionEncoder();
        outputArea.setText(getInfo());
        
    }
    
    public String getInfo(){
        return "Type in instructions separated by newlines:\n"+
                "ld r8, $27(r15)\n"+
                "add r6, r7, r1\n\n"+

                "And they will be encoded and displayed\nin the order they appear:\n"+
                "0C780027\n" +
                "1B388000\n\n"+

                "Use ; for comments\n"+
                "For lines that are only comments (beginning with ;), they will be just be passed through to output.\n"+
                "For example:\n" +
                ";ORG 0:\n"+
                "ldi r8, $27(r15);for my code\n\n"+

                "Machine Encoding Translation:\n"+
                "ORG 0:\n"+
                "0C780027 ;for my code\n\n"+

                "Checkbox option removes all appended comments, not comment lines (ie beginning with ;)\n\n"+

                "This message will disappear once you encode instructions\n"+
                "See again using Clear button";
                        
    }
        
    public void bulkEncode(){
        
        //String men = "add r1, r2, r3\nldi r6, $29(r7)";
        int lineCnt = 0;
        String commentChar = ";";
        if(inputArea.getText().equals("")){
            showMessageDialog(this, "No instruction to encode");
        } else{
            try{
                String[] input = inputArea.getText().split("\n");

                ArrayList<String> output = new ArrayList();

                for(String curline : input){
                    lineCnt++;
                    curline = curline.strip();
                    String curlineInst = "";
                    String curlineComment = "";
                    String outputComment = "";

                    if (curline.equals("")){
                        //empty line so just pass it through
                        output.add("");
                        continue;
                    }

                    //use regex to parse instruction and comments
                    Pattern r = Pattern.compile("(.*) *"+commentChar+"(.*)");
                    Matcher m = r.matcher(curline);
                    boolean thereAreComments = m.find();

                    if (thereAreComments){
                        //there is instruction and comment or there is only comment
                        curlineInst = m.group(1).strip();
                        curlineComment = m.group(2).strip();
                    } else{
                        //there is only instruction
                        curlineInst = curline;
                    }

                    //if the line is only a comment we should pass it through to the output
                    if ( curlineInst.equals("") ){
                        if (thereAreComments){
                            output.add(curlineComment);
                        }
                        continue;
                    }


                    //determine our output comment
                    //user wants both instruction and user comments
                    if(includeInst.isSelected() && includeComments.isSelected()) outputComment = curlineInst + ((curlineComment.equals("")) ? "" : ", "+curlineComment);

                    //user wants only instruction comments
                    else if(includeInst.isSelected() && !includeComments.isSelected()) outputComment = curlineInst;

                    //user wants only user comments
                    else if(!includeInst.isSelected() && includeComments.isSelected()) outputComment = curlineComment;

                    //now encode the instruction and add the output line
                    String encoded_instr = "";
                    try{
                        if (HBCombo.getSelectedItem().toString().equals("Binary")){
                            encoded_instr = instrEncoder.encodeInstruction(curlineInst, 2);
                        } else {
                           encoded_instr = instrEncoder.encodeInstruction(curlineInst);
                        }
                        output.add( encoded_instr +  ((outputComment.equals("")) ? "" : " "+commentChar + outputComment ) );
                    } catch (InstException e){
                        //if an exception occured, add the curlineInst as a comment with the exception message

                        output.add(commentChar+"ENCODING ERROR: "+curlineInst +", "+e.getMessage() );
                    }
                }

                setOutput(output);
            } catch(Exception e){
                String info = "Something went wrong (Line "+lineCnt+"):\n"
                        + e.getMessage()+"\n"
                        +"Are you sure your format is correct?";
                showMessageDialog(this, info);
            }
        }
        
    }
    
    public void setOutput(ArrayList<String> listIn){
        Iterator<String> myItr = listIn.iterator();
        String out = "";
        
        while(myItr.hasNext()){
           out = out + myItr.next()+"\n";
        }
        
        outputArea.setText(out);
    }
   
    
    
    public void clear(){
        String empty = "";
        
        inputArea.setText(empty);
        
       
       outputArea.setText(getInfo());
    }
    
    public void setInstrFormat(){
        formatField.setText( instrEncoder.getInstrFormat(nameField.getText().toLowerCase().split(" ")[0]));
    }
    

    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jTextField1 = new javax.swing.JTextField();
        jScrollPane1 = new javax.swing.JScrollPane();
        jTextPane1 = new javax.swing.JTextPane();
        title = new javax.swing.JLabel();
        jLabel1 = new javax.swing.JLabel();
        jSeparator1 = new javax.swing.JSeparator();
        jSeparator2 = new javax.swing.JSeparator();
        encodeBtn = new javax.swing.JButton();
        clearBtn = new javax.swing.JButton();
        HBCombo = new javax.swing.JComboBox<>();
        jLabel7 = new javax.swing.JLabel();
        jScrollPane2 = new javax.swing.JScrollPane();
        inputArea = new javax.swing.JTextArea();
        jScrollPane4 = new javax.swing.JScrollPane();
        outputArea = new javax.swing.JTextArea();
        jLabel2 = new javax.swing.JLabel();
        jScrollPane3 = new javax.swing.JScrollPane();
        formatField = new javax.swing.JTextArea();
        jLabel9 = new javax.swing.JLabel();
        nameField = new javax.swing.JTextField();
        jLabel3 = new javax.swing.JLabel();
        jLabel4 = new javax.swing.JLabel();
        jSeparator3 = new javax.swing.JSeparator();
        includeComments = new javax.swing.JCheckBox();
        includeInst = new javax.swing.JCheckBox();

        jTextField1.setText("jTextField1");

        jScrollPane1.setViewportView(jTextPane1);

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
        setTitle("Instruction Encoder");
        setResizable(false);

        title.setFont(new java.awt.Font("Dialog", 1, 24)); // NOI18N
        title.setText("Instruction Encoder");

        jLabel1.setFont(new java.awt.Font("Dialog", 1, 14)); // NOI18N
        jLabel1.setText("Instruction Name:");

        encodeBtn.setText("Encode Instructions");
        encodeBtn.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                encodeBtnMouseClicked(evt);
            }
        });

        clearBtn.setText("Clear");
        clearBtn.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                clearBtnMouseClicked(evt);
            }
        });
        clearBtn.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                clearBtnActionPerformed(evt);
            }
        });

        HBCombo.setModel(new javax.swing.DefaultComboBoxModel<>(new String[] { "Hex", "Binary" }));

        jLabel7.setFont(new java.awt.Font("Dialog", 1, 14)); // NOI18N
        jLabel7.setText("Output Format");
        jLabel7.setToolTipText("");

        inputArea.setColumns(20);
        inputArea.setRows(5);
        inputArea.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                inputAreaMouseClicked(evt);
            }
        });
        jScrollPane2.setViewportView(inputArea);

        outputArea.setEditable(false);
        outputArea.setColumns(20);
        outputArea.setLineWrap(true);
        outputArea.setRows(5);
        jScrollPane4.setViewportView(outputArea);

        jLabel2.setFont(new java.awt.Font("Dialog", 1, 14)); // NOI18N
        jLabel2.setText("Machine Encoding:");

        formatField.setEditable(false);
        formatField.setColumns(20);
        formatField.setRows(5);
        jScrollPane3.setViewportView(formatField);

        jLabel9.setText("Instruction format:");

        nameField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                nameFieldActionPerformed(evt);
            }
        });
        nameField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyReleased(java.awt.event.KeyEvent evt) {
                nameFieldKeyReleased(evt);
            }
        });

        jLabel3.setFont(new java.awt.Font("Dialog", 1, 14)); // NOI18N
        jLabel3.setText("Instructions (Newline Separated):");

        jLabel4.setFont(new java.awt.Font("Dialog", 1, 14)); // NOI18N
        jLabel4.setText("See Acceptable Formats Through Search");

        includeComments.setText("Include Comments");
        includeComments.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                includeCommentsActionPerformed(evt);
            }
        });

        includeInst.setText("Instruction Comments");
        includeInst.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                includeInstActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addGroup(layout.createSequentialGroup()
                        .addGap(21, 21, 21)
                        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(layout.createSequentialGroup()
                                .addComponent(jLabel4)
                                .addGap(0, 0, Short.MAX_VALUE))
                            .addComponent(jSeparator2)
                            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                    .addGroup(layout.createSequentialGroup()
                                        .addComponent(jLabel3)
                                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                                    .addGroup(layout.createSequentialGroup()
                                        .addComponent(jScrollPane2, javax.swing.GroupLayout.PREFERRED_SIZE, 234, javax.swing.GroupLayout.PREFERRED_SIZE)
                                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                                        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                            .addGroup(layout.createSequentialGroup()
                                                .addGap(0, 10, Short.MAX_VALUE)
                                                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                                                        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                                            .addComponent(encodeBtn, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.PREFERRED_SIZE, 148, javax.swing.GroupLayout.PREFERRED_SIZE)
                                                            .addComponent(clearBtn, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.PREFERRED_SIZE, 148, javax.swing.GroupLayout.PREFERRED_SIZE)
                                                            .addComponent(includeComments, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.PREFERRED_SIZE, 148, javax.swing.GroupLayout.PREFERRED_SIZE))
                                                        .addGap(20, 20, 20))
                                                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                                                        .addComponent(HBCombo, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                                                        .addGap(67, 67, 67))
                                                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                                                        .addComponent(jLabel7)
                                                        .addGap(42, 42, 42))))
                                            .addGroup(layout.createSequentialGroup()
                                                .addComponent(includeInst, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)))))
                                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                    .addComponent(jLabel2)
                                    .addComponent(jScrollPane4, javax.swing.GroupLayout.PREFERRED_SIZE, 245, javax.swing.GroupLayout.PREFERRED_SIZE))
                                .addGap(13, 13, 13))))
                    .addGroup(layout.createSequentialGroup()
                        .addContainerGap()
                        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(jSeparator1, javax.swing.GroupLayout.Alignment.TRAILING)
                            .addComponent(jSeparator3))))
                .addContainerGap())
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                .addGap(15, 15, 15)
                .addComponent(jLabel1)
                .addGap(18, 18, 18)
                .addComponent(nameField, javax.swing.GroupLayout.PREFERRED_SIZE, 138, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jLabel9)
                    .addComponent(jScrollPane3, javax.swing.GroupLayout.PREFERRED_SIZE, 351, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(19, 19, 19))
            .addGroup(layout.createSequentialGroup()
                .addGap(251, 251, 251)
                .addComponent(title, javax.swing.GroupLayout.PREFERRED_SIZE, 240, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(title)
                .addGap(2, 2, 2)
                .addComponent(jSeparator2, javax.swing.GroupLayout.PREFERRED_SIZE, 10, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel3)
                    .addComponent(jLabel2))
                .addGap(8, 8, 8)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                        .addComponent(jScrollPane4, javax.swing.GroupLayout.DEFAULT_SIZE, 261, Short.MAX_VALUE)
                        .addComponent(jScrollPane2))
                    .addGroup(layout.createSequentialGroup()
                        .addGap(19, 19, 19)
                        .addComponent(jLabel7)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(HBCombo, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(19, 19, 19)
                        .addComponent(encodeBtn, javax.swing.GroupLayout.PREFERRED_SIZE, 44, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addComponent(clearBtn, javax.swing.GroupLayout.PREFERRED_SIZE, 43, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addComponent(includeComments)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(includeInst)))
                .addGap(15, 15, 15)
                .addComponent(jSeparator1, javax.swing.GroupLayout.PREFERRED_SIZE, 4, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jLabel4)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jSeparator3, javax.swing.GroupLayout.PREFERRED_SIZE, 4, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jLabel9)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jScrollPane3, javax.swing.GroupLayout.PREFERRED_SIZE, 26, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(nameField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(jLabel1)))
                .addContainerGap(24, Short.MAX_VALUE))
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void encodeBtnMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_encodeBtnMouseClicked
        // TODO add your handling code here:
         bulkEncode();
    }//GEN-LAST:event_encodeBtnMouseClicked

    private void clearBtnMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_clearBtnMouseClicked
        // TODO add your handling code here:
        
        clear();
    }//GEN-LAST:event_clearBtnMouseClicked

    private void clearBtnActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_clearBtnActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_clearBtnActionPerformed

    private void nameFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_nameFieldActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_nameFieldActionPerformed

    private void nameFieldKeyReleased(java.awt.event.KeyEvent evt) {//GEN-FIRST:event_nameFieldKeyReleased
        // TODO add your handling code here:
        setInstrFormat();
    }//GEN-LAST:event_nameFieldKeyReleased

    private void includeCommentsActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_includeCommentsActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_includeCommentsActionPerformed

    private void includeInstActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_includeInstActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_includeInstActionPerformed

    private void inputAreaMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_inputAreaMouseClicked
        // TODO add your handling code here:
        
    }//GEN-LAST:event_inputAreaMouseClicked

    /**
     * @param args the command line arguments
     */
    

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JComboBox<String> HBCombo;
    private javax.swing.JButton clearBtn;
    private javax.swing.JButton encodeBtn;
    private javax.swing.JTextArea formatField;
    private javax.swing.JCheckBox includeComments;
    private javax.swing.JCheckBox includeInst;
    private javax.swing.JTextArea inputArea;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel7;
    private javax.swing.JLabel jLabel9;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JScrollPane jScrollPane2;
    private javax.swing.JScrollPane jScrollPane3;
    private javax.swing.JScrollPane jScrollPane4;
    private javax.swing.JSeparator jSeparator1;
    private javax.swing.JSeparator jSeparator2;
    private javax.swing.JSeparator jSeparator3;
    private javax.swing.JTextField jTextField1;
    private javax.swing.JTextPane jTextPane1;
    private javax.swing.JTextField nameField;
    private javax.swing.JTextArea outputArea;
    private javax.swing.JLabel title;
    // End of variables declaration//GEN-END:variables
}
