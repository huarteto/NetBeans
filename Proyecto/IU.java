/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package Proyecto;

import Proyecto.BC.tipoPiso;
import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.JComboBox;
import javax.swing.SpringLayout;

/**
 *
 * @author Diego
 */
public class IU extends javax.swing.JFrame {
    
    private JComboBox comboBox;
    private BC BaseConocimientoVenta;
    /**
     * Creates new form IU
     * 
     * Necesario cargar al menos una Base de Conocimiento.
     */
    public IU() throws IOException {
        initComponents();
        initBC();
    }
    /**
     * Inicializacion de una BC, identificada con las caracteristicas 
     *      por defecto.
     */
    private void initBC() {
        int[] limites = new int[3];
        limites[0] = 12;
        limites[1] = 12;
        limites[2] = 1000;
        
        
        try{
        BaseConocimientoVenta = new BC(limites);
        }catch(Exception e){
            textArea.setText("BC no se inicio correctamente.");
        }
    }

    /**
     * @deprecated 
     * @return
     * @throws FileNotFoundException
     * @throws IOException 
     */
    public String[] leerContenidosFichero() throws FileNotFoundException, IOException {
        String filePath = "";
        BufferedReader input = new BufferedReader(new FileReader(filePath));
        List<String> strings = new ArrayList<String>();
        try {
            String line = null;
            while ((line = input.readLine()) != null) {
                strings.add(line);
            }
        } catch (FileNotFoundException e) {
            System.err.println("Error, file " + filePath + " didn't exist.");
        } finally {
            input.close();

        }
        return strings.toArray(new String[]{});
        //String[] lineArray = strings.toArray(new String[]{});
    }

    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        menuBar1 = new java.awt.MenuBar();
        menu1 = new java.awt.Menu();
        menu2 = new java.awt.Menu();
        menuBar2 = new java.awt.MenuBar();
        menu3 = new java.awt.Menu();
        menu4 = new java.awt.Menu();
        buttonGroup1 = new javax.swing.ButtonGroup();
        buttonGroup2 = new javax.swing.ButtonGroup();
        jSpinner1 = new javax.swing.JSpinner();
        jButton1 = new javax.swing.JButton();
        buttonGroup3 = new javax.swing.ButtonGroup();
        TipoRecomendacion = new javax.swing.ButtonGroup();
        JTNHab = new java.awt.TextField();
        label1 = new java.awt.Label();
        label2 = new java.awt.Label();
        JTNWc = new java.awt.TextField();
        textArea = new java.awt.TextArea();
        TieneAscensor = new java.awt.Checkbox();
        label3 = new java.awt.Label();
        JTSuperficie = new java.awt.TextField();
        VPiso = new javax.swing.JRadioButton();
        jRadioButton2 = new javax.swing.JRadioButton();
        TargetVenta = new javax.swing.JComboBox<>();
        jButton2 = new javax.swing.JButton();
        jLabel1 = new javax.swing.JLabel();
        TipoVivienda = new javax.swing.JComboBox<>();
        jLabel2 = new javax.swing.JLabel();
        SliderRecomendacion = new javax.swing.JSlider();
        jLabel3 = new javax.swing.JLabel();
        Descuento = new java.awt.TextField();

        menu1.setLabel("File");
        menuBar1.add(menu1);

        menu2.setLabel("Edit");
        menuBar1.add(menu2);

        menu3.setLabel("File");
        menuBar2.add(menu3);

        menu4.setLabel("Edit");
        menuBar2.add(menu4);

        jButton1.setText("jButton1");

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
        setTitle("Recomendador precios piso");

        JTNHab.setText("00");
        JTNHab.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                JTNHabActionPerformed(evt);
            }
        });

        label1.setText("Num Habitaciones");

        label2.setText("Num Wc");

        JTNWc.setText("00");

        TieneAscensor.setLabel("Ascensor");

        label3.setText("Superficie");

        JTSuperficie.setText("0000");
        JTSuperficie.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                JTSuperficieActionPerformed(evt);
            }
        });

        TipoRecomendacion.add(VPiso);
        VPiso.setSelected(true);
        VPiso.setText("Venta piso");

        TipoRecomendacion.add(jRadioButton2);
        jRadioButton2.setText("Alquiler piso");
        jRadioButton2.setEnabled(false);

        TargetVenta.setModel(new javax.swing.DefaultComboBoxModel<>(new String[] { "Propietario 18-35 años", "Propietario 36- 50 años", "Propietario 51 - 100 años" }));
        TargetVenta.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                TargetVentaActionPerformed(evt);
            }
        });

        jButton2.setText("Pedir recomendación");
        jButton2.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                OnclickButton(evt);
            }
        });

        jLabel1.setText("Target venta");

        TipoVivienda.setModel(new javax.swing.DefaultComboBoxModel<>(new String[] { "Piso", "Duplex", "Atico", "Chalet", "Rustica" }));
        TipoVivienda.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                TipoViviendaActionPerformed(evt);
            }
        });

        jLabel2.setText("Tipo Vivienda");

        SliderRecomendacion.setMaximum(10000);
        SliderRecomendacion.setPaintLabels(true);
        SliderRecomendacion.setValue(0);
        SliderRecomendacion.addChangeListener(new javax.swing.event.ChangeListener() {
            public void stateChanged(javax.swing.event.ChangeEvent evt) {
                SliderRecomendacionStateChanged(evt);
            }
        });

        jLabel3.setText("Desviacion recomendacion");

        Descuento.setText("000.0");
        Descuento.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                DescuentoActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addGroup(layout.createSequentialGroup()
                        .addGap(52, 52, 52)
                        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(jRadioButton2)
                            .addGroup(layout.createSequentialGroup()
                                .addComponent(VPiso)
                                .addGap(77, 77, 77)
                                .addComponent(TieneAscensor, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                            .addGroup(layout.createSequentialGroup()
                                .addGap(152, 152, 152)
                                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                    .addComponent(jLabel3)
                                    .addComponent(SliderRecomendacion, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))))
                        .addGap(0, 0, Short.MAX_VALUE))
                    .addGroup(layout.createSequentialGroup()
                        .addGap(22, 22, 22)
                        .addComponent(textArea, javax.swing.GroupLayout.PREFERRED_SIZE, 620, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addContainerGap(31, Short.MAX_VALUE))
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addGroup(layout.createSequentialGroup()
                        .addComponent(jButton2)
                        .addGap(267, 267, 267))
                    .addGroup(layout.createSequentialGroup()
                        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(label1, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(JTNHab, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addGap(46, 46, 46)
                        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(label2, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(JTNWc, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addGap(70, 70, 70)
                        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                            .addComponent(label3, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(JTSuperficie, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addGap(72, 72, 72)
                        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(jLabel2)
                            .addComponent(TargetVenta, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(jLabel1)
                            .addComponent(TipoVivienda, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))))
                .addGap(57, 57, 57))
            .addGroup(layout.createSequentialGroup()
                .addGap(273, 273, 273)
                .addComponent(Descuento, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addGap(38, 38, 38)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(layout.createSequentialGroup()
                        .addGap(2, 2, 2)
                        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(label1, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(label2, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)))
                    .addComponent(label3, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(jLabel1))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(JTNHab, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(JTNWc, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(TargetVenta, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(JTSuperficie, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(layout.createSequentialGroup()
                        .addGap(37, 37, 37)
                        .addComponent(jLabel2)
                        .addGap(18, 18, 18)
                        .addComponent(TipoVivienda, javax.swing.GroupLayout.PREFERRED_SIZE, 20, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(layout.createSequentialGroup()
                        .addGap(55, 55, 55)
                        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(TieneAscensor, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(VPiso))))
                .addGap(18, 18, 18)
                .addComponent(jRadioButton2)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addComponent(jLabel3)
                .addGap(18, 18, 18)
                .addComponent(SliderRecomendacion, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(Descuento, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(36, 36, 36)
                .addComponent(jButton2)
                .addGap(0, 0, 0)
                .addComponent(textArea, javax.swing.GroupLayout.PREFERRED_SIZE, 153, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(29, 29, 29))
        );

        JTNHab.getAccessibleContext().setAccessibleName("JTNHab");
        JTNHab.getAccessibleContext().setAccessibleDescription("JTNHab");
        JTNWc.getAccessibleContext().setAccessibleName("JTNWc");
        TieneAscensor.getAccessibleContext().setAccessibleName("CBAscensor");

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void TargetVentaActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_TargetVentaActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_TargetVentaActionPerformed

    private void OnclickButton(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_OnclickButton
        PisoDAO aux00 = null;
        Integer[] argsInt = new Integer[3];
        Double[] argsDouble = new Double[1]; 
        Boolean[] argsBoolean = new Boolean[1];
        tipoPiso tipo = null;
        int target = TargetVenta.getSelectedIndex();
        double descuento = SliderRecomendacion.getValue();
        if(descuento > 0){
            descuento = descuento * 0.01;
        }
        
        argsInt[0] = Integer.parseInt(JTNHab.getText());
        argsInt[1] = Integer.parseInt(JTNWc.getText());
        argsInt[2] = Integer.parseInt(JTSuperficie.getText());

        argsBoolean[0] = TieneAscensor.getState();
        argsDouble[0] = null;
        
        
        if (argsInt[2] != 0) {
            tipo = tipoPiso.valueOf(TipoVivienda.getSelectedItem().toString());
//        System.out.println(TargetVenta.getSelectedItem().toString());
            aux00 = new PisoDAO(argsInt, argsDouble, argsBoolean, tipo);
//        System.out.println(descuento);+
            System.out.println(">>>" + aux00.toSgring());
            double recomendacion = BaseConocimientoVenta.pedirRecomendacion(aux00, descuento, target);
            recomendacion = recomendacion * 1000;
            textArea.setText("Precio recomendao: " + recomendacion);
        }else{
            textArea.setText("No se puede cotizar una superficie 0.");
        }
//        tipo = 
    }//GEN-LAST:event_OnclickButton

    private void TipoViviendaActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_TipoViviendaActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_TipoViviendaActionPerformed

    private void JTNHabActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_JTNHabActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_JTNHabActionPerformed

    private void DescuentoActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_DescuentoActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_DescuentoActionPerformed

    private void SliderRecomendacionStateChanged(javax.swing.event.ChangeEvent evt) {//GEN-FIRST:event_SliderRecomendacionStateChanged
        // TODO add your handling code here:
        Double aux = SliderRecomendacion.getValue() * 0.01;
        String aux00 = String.format("%.2f", aux);
        Descuento.setText(aux00+"\n");
    }//GEN-LAST:event_SliderRecomendacionStateChanged

    private void JTSuperficieActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_JTSuperficieActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_JTSuperficieActionPerformed

    /**
     * @param args the command line arguments
     */
    public static void main(String args[]) {
        /* Set the Nimbus look and feel */
        //<editor-fold defaultstate="collapsed" desc=" Look and feel setting code (optional) ">
        /* If Nimbus (introduced in Java SE 6) is not available, stay with the default look and feel.
         * For details see http://download.oracle.com/javase/tutorial/uiswing/lookandfeel/plaf.html 
         */
        try {
            for (javax.swing.UIManager.LookAndFeelInfo info : javax.swing.UIManager.getInstalledLookAndFeels()) {
                if ("Nimbus".equals(info.getName())) {
                    javax.swing.UIManager.setLookAndFeel(info.getClassName());
                    break;
                }
            }
        } catch (ClassNotFoundException ex) {
            java.util.logging.Logger.getLogger(IU.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (InstantiationException ex) {
            java.util.logging.Logger.getLogger(IU.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (IllegalAccessException ex) {
            java.util.logging.Logger.getLogger(IU.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (javax.swing.UnsupportedLookAndFeelException ex) {
            java.util.logging.Logger.getLogger(IU.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        }
        //</editor-fold>

        /* Create and display the form */
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                try {
                    new IU().setVisible(true);
                } catch (IOException ex) {
                    Logger.getLogger(IU.class.getName()).log(Level.SEVERE, null, ex);
                }
            }
        });
    }

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private java.awt.TextField Descuento;
    private java.awt.TextField JTNHab;
    private java.awt.TextField JTNWc;
    private java.awt.TextField JTSuperficie;
    private javax.swing.JSlider SliderRecomendacion;
    private javax.swing.JComboBox<String> TargetVenta;
    private java.awt.Checkbox TieneAscensor;
    private javax.swing.ButtonGroup TipoRecomendacion;
    private javax.swing.JComboBox<String> TipoVivienda;
    private javax.swing.JRadioButton VPiso;
    private javax.swing.ButtonGroup buttonGroup1;
    private javax.swing.ButtonGroup buttonGroup2;
    private javax.swing.ButtonGroup buttonGroup3;
    private javax.swing.JButton jButton1;
    private javax.swing.JButton jButton2;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JRadioButton jRadioButton2;
    private javax.swing.JSpinner jSpinner1;
    private java.awt.Label label1;
    private java.awt.Label label2;
    private java.awt.Label label3;
    private java.awt.Menu menu1;
    private java.awt.Menu menu2;
    private java.awt.Menu menu3;
    private java.awt.Menu menu4;
    private java.awt.MenuBar menuBar1;
    private java.awt.MenuBar menuBar2;
    private java.awt.TextArea textArea;
    // End of variables declaration//GEN-END:variables
}
