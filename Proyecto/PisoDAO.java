package Proyecto;

/**
 * DAO de nuestro caso de estudio.
 * 
 * @author Diego Alvarez
 * @author Javier Bielsa
 * 
 * @version 1.0
 * @since 30/12/2016
 * Se ha echo la clasee DAO mas generica.
 * 
 * @version 1.1
 * @since 08/01/2017
 * Añadido tratamiento de errores para el aceso a las variables almacenadas en 
 * Arrays, que podian lanzar un IndexOutOfBoundsException.
 * 
 */
public class PisoDAO<Id01 extends Enum>{

    /**
     * Campos enteros
     */
    private Integer[] camposEnteros; //n hab, n wc, superficie
    private Double[] camposDouble;  //Precio
    private Boolean[] camposBoolean; //Ascensor
    private Id01 campoEnum;     //tipo
    private String[] camposSrings;   //calle?

    private PisoDAO[] sigEleEnteros;
    private PisoDAO[] sigEleDoubles;
   
    
    public PisoDAO(Integer _enteros[], Double _dobles[], Boolean _boleanos[], Id01 _enum) {
        this.camposEnteros = _enteros;
        this.camposDouble = _dobles;
        this.camposBoolean = _boleanos;
        
        this.campoEnum = _enum;
        
        sigEleEnteros = new PisoDAO[_enteros.length];
        sigEleDoubles = new PisoDAO[_dobles.length];
    }
    
    
    
    /**
     * Devuelve un entero almacenado. O -37 en caso de _index no correcto.
     * 
     * @param _index
     * @return 
     */
    public Integer getCampoEntero(int _index) {
        Integer respuesta = null;
            try{
                respuesta = camposEnteros[_index];
                
            }catch(IndexOutOfBoundsException ex){
                respuesta = Integer.MAX_VALUE;
            }finally{
                return respuesta;
            }
    }
    public int getCamposEnteroSize(){
        return camposEnteros.length;
    }
    
    /**
     * Devuelve un double almacenado. O -37.0 en caso de _index no correcto.
     * 
     * @param _index
     * @return 
     */
    public Double getCampoDouble(int _index) {
        Double respuesta = null;
        try{
            respuesta = camposDouble[_index];
        }catch(IndexOutOfBoundsException ex){
            respuesta = Double.NaN;
        }finally{
            return respuesta;
        }
    }
    public int getCamposDoubleSize(){
        return camposDouble.length;
    }
    
    /**
     * PRE: eres responsalbe de _index. 
     * Devuelve un double almacenado
     *
     * @param _index
     * @return
     */
    public boolean getCampoBoolean(int _index) {
        Boolean respuesta = null;
        try {
            respuesta = camposBoolean[_index];

        } catch (IndexOutOfBoundsException ex) {
            respuesta = null;
        } finally {
            return respuesta;
        }
    }
    public int getCamposBooleanSize(){
        return camposBoolean.length;
    }
   
    /**
     * Devuelve un Enum almacenado.
     * 
     * @return 
     */
    public Id01 getCampoEnum() {
        return campoEnum;
    }   
    
    /**
     * Devuelve un string almacenado. O null en caso de _index no correcto.
     * 
     * @param _index
     * @return 
     */
    public String getCampoSring(int _index) {
        String respuesta = null;
        try{
            respuesta = camposSrings[_index];
        }catch(IndexOutOfBoundsException ex){
            respuesta = null;
        }finally{
            return respuesta;
        }
    }
    public int getCamposStringSize(){
        return camposSrings.length;
    }

    
    /**
     * Devuelve el sig PisoDao el cual esta ordenado en _index con este PisoDao
     * O null en caso de _index no correcto.
     * @param _index
     * @return 
     */
    public PisoDAO getSigEleEntero(int _index) {
        PisoDAO respuesta = null;
        try {
            respuesta = sigEleEnteros[_index];
        } catch (IndexOutOfBoundsException ex) {
            respuesta = null;
        } finally {
            return respuesta;
        }
    }
    public void setSigEleEntero(int _index, PisoDAO _sigEle) {
        this.sigEleEnteros[_index] = _sigEle;
    }
    
    /**
     * Devuelve el sig PisoDao el cual esta ordenado en _index con este PisoDao
     * O null en caso de _index no correcto.
     * @param _index
     * @return
     */
    public PisoDAO getSigEleDouble(int _index) {
        PisoDAO respuesta = null;
        try{
            respuesta = sigEleDoubles[_index];
        }catch(IndexOutOfBoundsException ex){
            respuesta = null;
        }finally{
            return respuesta;
        }
    }
    public void setSigEleDouble(int _index, PisoDAO _sigEle){
        this.sigEleDoubles[_index] = _sigEle;
    }
    


    /**
     * 
     * @param _index
     * @param _otroInt
     * @return 
     * @throws IndexOutOfBoundsException Cuidado con _index.
     */
    public int compareEntero(int _index, int _otroInt) throws IndexOutOfBoundsException{
       return this.camposEnteros[_index].compareTo(_otroInt);
    }
    /**
     * 
     * @param _index
     * @param _otroDouble
     * @return
     * @throws IndexOutOfBoundsException Cuidado con _index.
     */
    public int compareDoble(int _index, double _otroDouble)throws IndexOutOfBoundsException{
        return this.camposDouble[_index].compareTo(_otroDouble);
    }
    public boolean isEnum(Id01 _otroenum){
       return this.campoEnum.compareTo(_otroenum) == 0;
    }

    
    public String toSgring(){
        String respuesta = "";
        int j = 0;
        while(j < camposEnteros.length){
            respuesta += camposEnteros[j] + " ; ";
            j++;
        }
        j = 0;
        while(j  < camposBoolean.length){
            respuesta += camposBoolean[j] + " ; ";
            j++;
        }
        respuesta += campoEnum.toString() + " ; ";
        j = 0;
        while (j < camposDouble.length) {
            respuesta += camposDouble[j] + " ; ";
            j++;
        }
        
        return respuesta;
    }
}
