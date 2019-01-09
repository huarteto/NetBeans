package Proyecto;

import Proyecto.PisoDAO;
import java.io.IOException;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.lang.ExceptionInInitializerError;

/**
 * Clase que sera la Base de conocimiento.
 *
 * @author Diego Alvarez
 * @author Javier Bielsa
 * 
 * @version 1.0
 * @since 08/12/2016
 * 
 * @version 2.0
 * @since 22/12/2016
 * Muerte y destruccion MAP de MAPS
 * 
 * @version 2.1
 * @since 08/01/2017
 * Tras la modificacion del PisoDAO V.1.1 se corrige los codigos previos que 
 * comprueben los datos recibidos de PisoDao. Y se añade un metodo deleteOnBC
 * para mantener la estabilidad de la Base de Conocimiento.
 * 
 */
public class BC {



    private Map<Integer,Map<Integer, PisoDAO>> mapas;
    private int elementosAlmacenados;
    

    /* Precio medio de elementos con SI ascensor  */
    private double PRECIO_YES_ASCENSOR = 0;
    /* Precio medio de elementos con NO ascensor  */
    private double PRECIO_NO_ASCENSOR = 0;
    enum tipoPiso {
        Piso, Duplex, Atico, Chalet, Rustica
    }

    private double[] PESO_CAMPO; //hab, wc, superficie, ascnesor
    private double TIPO_COINCIDE = 0.6;
    private double TIPO_NO_COINCIDE = 0.1;
    private int CAMPO_SUPERFICIE = 3;

    public final static String[] rutasArchivosCSV = {"./src/Proyecto/Viviendas_nuevas.csv",
        "./Viviendas_nuevas_Actur.csv"};
    /**
     * Codificacion de caracteres *
     */
    public final static Charset ENCODING = StandardCharsets.ISO_8859_1;
    /**
     * Delimitador *
     */
    public final static String delimitador = ";"; 

    
    public BC(int[] _limits) {
        int init;
        initialization(_limits); 
        init = importarCasosDesdeListaRutas(rutasArchivosCSV);
        
        if(init == 0){
            throw (new ExceptionInInitializerError());
        }
    }

    
    /**
     * OJO a los numeros limite de los bucles, podria dar error.
     *
     */
    private void initialization(int[] _limits) {
        int i = 0;
        int j = 0;
        Map<Integer, PisoDAO> aux;
        mapas = new HashMap<Integer, Map<Integer, PisoDAO> >();
        
        
        while (j < _limits.length) {
            i = 0;
            aux = new HashMap<Integer, PisoDAO>();
            do {
                aux.put(i, null);
                i++;
            } while (i < _limits[j]);
            
            mapas.put(j, aux);
            j++;
        }
        
        this.elementosAlmacenados = 0;

        PESO_CAMPO = FieldsLoad.getLoad(0);        
    }

    protected void insertar(PisoDAO _e)throws IndexOutOfBoundsException {
        int i = 0;
        try {
            // System.out.println("Flag 0A.");
            while (i < _e.getCamposEnteroSize()) {
                insercion(i, _e);
                i++;
            }

            insercionAscensor(_e);

            this.elementosAlmacenados++;
        } catch (IndexOutOfBoundsException ex) {
            //Error on Insertion.
            //Not necessary notify.
            deleteOnBC(_e, i);            
        }
    }
    private void insercion(int _campoInsercion, PisoDAO _e) throws IndexOutOfBoundsException {
        Map<Integer, PisoDAO> mapaAux = mapas.get(_campoInsercion);
        int posicion = (int) _e.getCampoEntero(_campoInsercion);
        if(_campoInsercion == (CAMPO_SUPERFICIE-1) ){
            posicion = posicion / 100;
        }
        PisoDAO aux01 = mapaAux.get(posicion);
        PisoDAO aux02;

        if (aux01 == null) {
            mapaAux.put(posicion, _e);
        }else if(_e.compareEntero(_campoInsercion, 
                aux01.getCampoEntero(_campoInsercion)) < 0){
            
            mapaAux.put(posicion, _e);
            _e.setSigEleEntero(_campoInsercion, aux01);
        } else {
            do {
                aux02 = aux01.getSigEleEntero(_campoInsercion);
                if (null == aux02) {
                    aux01.setSigEleEntero(_campoInsercion, _e);
                    break;
                } else {
                    if(_e.compareEntero(_campoInsercion, 
                            aux02.getCampoEntero(_campoInsercion)) < 0){
                        aux01.setSigEleEntero(_campoInsercion, _e);
                        _e.setSigEleEntero(_campoInsercion, aux02);
                        break;
                    }
                    aux01 = aux02;
                }
            } while (true);
        }
    }
    /**
     * Estimacion del coste de ascensor.
     * 
     * @param _e 
     */
    private void insercionAscensor(PisoDAO _e) {
        double auxprecio;
        
        if ( _e.getCampoBoolean(0)) {
            auxprecio = _e.getCampoDouble(0) * PESO_CAMPO[3];
            PRECIO_YES_ASCENSOR += auxprecio;
            if (PRECIO_YES_ASCENSOR != auxprecio) {
                PRECIO_YES_ASCENSOR = PRECIO_YES_ASCENSOR / 2;
            }
        } 
        //---------------------Podria Sobrar----------------------------------*
        else {                                                              //*
            auxprecio = _e.getCampoDouble(0) * PESO_CAMPO[3];               //*
            PRECIO_NO_ASCENSOR += auxprecio;                                //* 
            if (PRECIO_NO_ASCENSOR != auxprecio) {                          //*
                PRECIO_NO_ASCENSOR = PRECIO_NO_ASCENSOR / 2;                //*
            }                                                               //*
        }//-------------------------------------------------------------------*
    }
    
    /**
     * En el caso de querer borrar un elemento de la Base de Conocimiento.
     * 
     * @param _e
     * @param _limit 
     */
    private void deleteOnBC(PisoDAO _e, int _limit){
        int j = 0;
        while(j < _limit){
            borrado(j, _e);
            j++;
        }
    }
    /**
     * No se hace tratamiento de limties de indices de almacenamiento, porque 
     *      sabemso que la ejecucion para eos parametros es correcta.
     * 
     * @param _campoBorrado
     * @param _e 
     */
    private void borrado(int _campoBorrado, PisoDAO _e) {
        Map<Integer, PisoDAO> mapaAux = mapas.get(_campoBorrado);
        int posicion = (int) _e.getCampoEntero(_campoBorrado);
        if (_campoBorrado == (CAMPO_SUPERFICIE - 1)) {
            posicion = posicion / 100;
        }
        PisoDAO aux01 = mapaAux.get(posicion);
        PisoDAO aux02;
        


        if (aux01.equals(_e)) {
            mapaAux.put(posicion, _e.getSigEleEntero(_campoBorrado));

        } else {
            
            aux02 = aux01.getSigEleEntero(_campoBorrado);

            /**
             * Condicion de bucle para evitar bucles infinitos en un hipotetico
             * caso aunque nada posible caso de que se busque un elemento
             * que sabemso que se ha añadido a la coleccion y tras haber 
             * recorrido todo el espacio sin encontrarlo saltaria 
             * un NullPointerException.
             */            
            while (aux02 != null){
                if (aux02.equals(_e)) {
                    aux01.setSigEleEntero(_campoBorrado,
                            aux02.getSigEleEntero(_campoBorrado));
                    break;
                }
                aux02 = aux01.getSigEleEntero(_campoBorrado);
            } 
        }
    }

    /**
     * Para pedir la recomendacion basandonos en los elemenos de la Base de
     * conocimiento.
     *
     * @param _e PisoDAO
     * @param _descuento double
     * @param _target int
     * @return precio asignado
     */
    protected double pedirRecomendacion(PisoDAO _e, double _descuento, int _target) {
        double respuesta = -4;
        double auxCoste = 0.0;
        double auxRecal = 0.0;
        int j = 0;
        int auxIndex = _e.getCamposEnteroSize();
        PESO_CAMPO = FieldsLoad.getLoad(_target);

        if (this.elementosAlmacenados == 0) {
            respuesta = -2;
        } else {
            respuesta = 0;
            while(j < auxIndex){
                auxCoste = recomenDimension(_e, j);
                if(_descuento <= 0.0){
                    respuesta += auxCoste;
                }else{
                    respuesta += auxCoste * (1 - _descuento);
                }
                j++;
            }
            
            if(_e.getCampoBoolean(0)){
                auxRecal = PESO_CAMPO[3] / FieldsLoad.getDefault();
                respuesta += auxRecal * PRECIO_YES_ASCENSOR;
            }

            //Ir añadiendo en caso de dimensiones no cuantificativas.
        }

        return respuesta;
    }
    
    /**
     * Recomendacion en Dimension.
     * 
     * Estima el precio recomendado basandonos en una dimension. En el caso de que
     * no haya elementos en la Base de Conocimiento que coincidan en esa dimension,
     * recorreremos los adyacentes hasta hacer una media.
     *
     * @param _e PisoDAO
     *
     * @return precio estimado
     */
    private double recomenDimension(PisoDAO _e, int _dimension) {
        double respuestaCoste = 0;
        int numBuscando;       //Por lo que vamos a buscar
        double costeAux01;     //Precio de elementos inferiores.
        double costeAux02;     //Precio de elementos superiores.
        double costeAux03;     //Precio de elementos adyacentes.
        int nBaux01;
        int nBaux02;

        numBuscando = (int) _e.getCampoEntero(_dimension);
        if (numBuscando != Integer.MAX_VALUE) {
            if(_dimension == 2){
                numBuscando = numBuscando / 100;
            }
            respuestaCoste = costeDIguales(numBuscando, _dimension, (tipoPiso) _e.getCampoEnum());
            //En el caso de que no haya elementos con el mismo numero en ese
            // campo vamos a hacer una media con los adyacentes.
            if (respuestaCoste == 0) {
                nBaux01 = numBuscando - 1;
                costeAux01 = 0;
                while (costeAux01 != 0 && nBaux01 >= 0) {
                    costeAux01 = costeDIguales(nBaux01, _dimension, (tipoPiso) _e.getCampoEnum());
                    nBaux01--;
                }

                nBaux02 = numBuscando + 1;
                costeAux02 = 0;
                while (costeAux02 != 0 && nBaux02 < mapas.get(_dimension).size()) {
                    costeAux02 = costeDIguales(nBaux02, _dimension, (tipoPiso) _e.getCampoEnum());
                    nBaux02++;
                }

                costeAux03 = costeAux01 + costeAux02;
                if (costeAux01 != 0 && costeAux02 != 0) {
                    costeAux03 = costeAux03 / 2; 
                }

                respuestaCoste = costeAux03;
            }
        }

        return respuestaCoste;
    }
    private double costeDIguales(int _num, int _dimension, tipoPiso _tipo) {
        PisoDAO aux00 =  mapas.get(_dimension).get(_num);  
        double respuesta = 0;
        double costeNPisos =  0;
        double costePiso;
        
        int nelementos = 0;

        if (aux00 != null) {
            do {
                costePiso = PESO_CAMPO[_dimension] *  aux00.getCampoDouble(0);
                if(_tipo == (tipoPiso) aux00.getCampoEnum()){
                    costePiso = costePiso * TIPO_COINCIDE;
                }else{
                    costePiso =  costePiso * TIPO_NO_COINCIDE;
                }
                
                costeNPisos += costePiso;
                
                
                nelementos++;
                aux00 = aux00.getSigEleEntero(_dimension);
//                System.out.println(nelementos + "----: " + PESO_CAMPO01 * 
//                        (int) aux00.getInformacion() + ">>>"
//                        + aux00.getInformacion());
            } while (aux00 != null);

            if (nelementos > 0) {
                respuesta = costeNPisos / nelementos;
            }
        }

        return respuesta;
    }

    protected void listar() {
        int j = 0;
        int auxIndex = mapas.size();
        
        while (j < auxIndex) {
            listarDimension(j);
            j++;
        }
    }
    private void listarDimension(int _dimension) {
        Map<Integer, PisoDAO> mapaAux = mapas.get(_dimension);
        PisoDAO aux;
        int n = 0;

        if (!mapaAux.isEmpty()) {
//            System.out.println("Listado campo 0" + _dimension);
            do {
                aux = mapaAux.get(n);
                if (aux != null) {
                    do {
                        System.out.println(aux.toSgring());
                        aux = aux.getSigEleEntero(_dimension);
                    } while (aux != null);
                }
                n++;
            } while (n < mapaAux.size());
        } else {
//            System.err.println("Error inseerccion 01");
        }
    }

    /**
     * Importar los casos agregados desde un archivo CSV.
     *
     * @param _ruta del archivo CSV
     * @return Determina se se ha importado los casos con exito
     */
    public boolean importarCasos(String _ruta){
        String linea;
        String[] argumentos;
        PisoDAO aux;
        //---------------------Variables para Insercion---------------
        tipoPiso tipo;
        int nHab, nWc, superficie;
        Integer[] argsInt;
        Double[] argsDouble;
        Boolean[] argsBoolean;
        
        double precio;
        boolean ascensor;
        //-----------------------------------------
        
        try{
            // Extraer las lineas dle archivo
            Path path = Paths.get(_ruta);
            List<String> lineas = Files.readAllLines(path, ENCODING);
            Iterator<String> it = lineas.iterator();
            // Ignorar la primera linea con nombres de columnas
            it.next();
            //Para cada Linea crear un caso.

            while( it.hasNext() ){
                linea = it.next();
//                System.out.println(linea);
                argumentos = linea.split(delimitador);
                argsInt = new Integer[3];
                argsDouble = new Double[1];
                argsBoolean = new Boolean[1]; 
                //Alquileres necesitaemos 2 ----------------------MODIFICAR
                
                if( argumentos.length > 0 ){
                    tipo = tipoPiso.valueOf(argumentos[2]);

                    argsInt[0] = Integer.parseInt(argumentos[5]);
                    argsInt[1] = Integer.parseInt(argumentos[6]);
                    argsInt[2] = Integer.parseInt(argumentos[9]);

                    argsBoolean[0] = argumentos[7].equals("si");
                    argsDouble[0] = Double.parseDouble(argumentos[8]);


                    aux = new PisoDAO(argsInt, argsDouble, argsBoolean,  tipo);
                    System.out.println("Insertando: " + aux.toSgring());
                    insertar(aux);
                }
            }
            
            return true;
        }catch(IOException e){
//            System.out.println("Error al cargar archivos: " + e.getMessage());
//            e.printStackTrace();
            return false;
        }
    }
    
    /**
     * Prueba a importar casos desde una lista de rutas, hasta que una
     *      de ellas se imporata el archivo CSV con exito.
     * 
     * @param _rutas lista de rutas posibles de los archivos CSV    
     * @return 1 - Se importo correctamente. 0 - Error cargando Base Conocimeinto
     */
    public int importarCasosDesdeListaRutas(String[] _rutas){
        boolean exito = false;
        int i = 0;
        int respuesta = 0;
        
        while(i < _rutas.length && !exito){
            exito = importarCasos(_rutas[i]);
            i++;
        }
        
        if(exito){
            respuesta = 1;
//            System.out.println("Se ha cargado la base de conocimiento desde archivo");
        }else{
            respuesta = 0;
//            System.out.println("Ha habido un error  cargando la base de conocimiento");
        }
        
        return respuesta;
    }
}
