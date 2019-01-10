package Modelo;

import java.text.DecimalFormat;
import java.util.GregorianCalendar;

/**
 *
 * @author Guillermo
 * 
 * Constructor de coche, aú/nn hay que poner las variables reales.
 * 
 */
public class Coche implements Comparable<Coche>{
    final double MAXCOMODIDAD_PRECIO = 0.3;
    final double MAXCOMODIDAD_PUERTAS = 0.35;
    final double MAXCOMODIDAD_CAJACAMBIOS = 0.35;
    
    final double MINTIEMPOVIAJE_CABALLOS = 0.65;
    final double MINTIEMPOVIAJE_PESO = 0.15;
    final double MINTIEMPOVIAJE_MOTOR = 0.15;
    
    String concesionario;
    String tipo_Coche;
    String color;
    String modelo;
    String marca;
    int puertas, caballos, id, peso;
    int precio;
    boolean manual, gasolina;
    double valor = 0;
    
    
    /**
     * Se crean coches a partir de lo leido por el csv mediante una cadena y split
     * @param linea 
     */
    public Coche(String linea, int id){
        String[] cadena = linea.split(";");
        this.id = id;
        for(int i = 0; i < cadena.length; i++){
            this.concesionario = cadena[0];
            this.marca = cadena[1];
            this.tipo_Coche = cadena[2];
            this.color = cadena[3];
            this.modelo = cadena[4];
            this.puertas = Integer.parseInt(cadena[5]);
            this.caballos = Integer.parseInt(cadena[6]);
            if(cadena[7].equals("si")) this.manual = true;
            else{this.manual = false;}
            this.precio = Integer.parseInt(cadena[8]);
            this.peso = Integer.parseInt(cadena[9]);
            if(cadena[10].equals("si")) this.gasolina = true;
            else{this.gasolina = false;}
        }
    }
    
    public String automatico(){
        if(manual) return "No";
        return "Si";
    }
    
    public String gasolina(){
        if(gasolina) return "Si";
        return "No";
    }
    
    public int devuelvePuertas(){
        return puertas;
    }
    
    public int devuelvePeso(){
        return peso;
    }
    
    public int devuelveId(){
        return this.id;
    }

    public int devuelveIdLista(int index){
        id = index;
        return id;
    }
    
    @Override
    public String toString() {
        return " Coche " + tipo_Coche + ", color=" + color + ", modelo=" + modelo
                + ", marca=" + marca + ", puertas=" + puertas + ", caballos=" + 
                caballos + ", id=" + id + ", peso=" + peso + ", precio=" + precio
                + ", manual=" + manual + ", gasolina=" + gasolina + " \n";
    }
    
    public String devuelveTipo(){
        return tipo_Coche;
    }
    
        public String devuelveModelo(){
        return modelo;
    }
        public String devuelveMarca(){
            return marca;
        }
        
        public int compareTo(Coche o) {
            if (precio < o.precio) {
                return -1;
            }
            if (precio > o.precio) {
                return 1;
            }
            return 0;
        }
        
        public int devuelvePrecio(){
            return precio;
        }
        /**
         * Precio añadiendo si es automatico
         * @param suplemento
         * @return 
         */
        public int devuelvePrecio(int suplemento){
            return precio+suplemento;
        }
        
        public int devuelveCaballos(){
            return caballos;
        }
        
        public int devuelveNumPuertas(){
            return puertas;
        }
        
        public double devuelveValor(){
            return valor;
        }
        
        public String devuelveValor2Decimales(){
           DecimalFormat df = new DecimalFormat("#.00");
            return  df.format(valor);
        }
        
        
        /**
         * Metodos para el calculo del algoritmo
         * @param Min
         * @param Max 
         */
        public void calcularMinPrecio(int Min, int Max){
            float precio_total = Max-Min;
            this.valor = 100-((((this.precio-Min)*100)/precio_total));
        }
        
        public void calcularMaxComodidad(int Min, int Max){
            double precio_total = Max-Min;
            double valor = (100-((((this.precio-Min)*100)/precio_total)))
                    *MAXCOMODIDAD_PRECIO;
            if(!manual) valor = valor + MAXCOMODIDAD_CAJACAMBIOS;
            if(puertas == 5) valor = valor + MAXCOMODIDAD_PUERTAS;
            this.valor = valor;
        }
        
        public void calcularMinCosteViaje(int MinP, int MaxP, int MinC, int MaxC){
            double peso_total = MaxP-MinP;
            double caballos_total = MaxC-MinC;
            double valor = (100-((((this.peso-MinP)*100)/peso_total)))
                    *MINTIEMPOVIAJE_PESO;
            valor = valor + ((100-((((this.caballos-MinC)*100)/caballos_total)))
                    *MINTIEMPOVIAJE_CABALLOS);
            if(gasolina) valor = valor + MINTIEMPOVIAJE_MOTOR;
            this.valor = valor;
            
        }
    }
    

