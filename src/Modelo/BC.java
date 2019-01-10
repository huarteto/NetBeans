/*/
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package Modelo;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author Guillermo Tipo de decision: Incertidumbre : las consecuencias
 * derivadas de cada cada alternativa no se conocen con exactitud. Estas
 * consecuencias pueden llegar a ser previstas o no --> Estadistica/Probabilidad
 *
 * Diferencia manual / automatico oscilar entre 1.000 y 2.500 euros de media
 * Gasolina media 2000 mas barato que diesel Segundo mano --> 30/40 % menos
 *
 * jdbc:sqlserver://lenovo-alvaro.database.windows.net:1433;databaseName=SATD2018
 */
public class BC {

    private final Map lista;
    public Map aux;
    boolean firstLine = true;
    private int PRECIO_MIN = 10000;
    private int PRECIO_MAX = 0;
    private int PESO_MIN = 10000;
    private int PESO_MAX = 0;
    private int CABALLOS_MIN = 10000;
    private int CABALLOS_MAX = 0;
    private ArrayList listaCoche;
    Random rand = new Random();
    public final static String rutaCSV = "C:\\Users\\Guillermo\\Desktop\\ACTUAL\\SATD\\Coches.csv";
    /**
     * SELECT * FROM dbo.Coches;
     */
    private Statement stm;
    private ResultSet RS, con, mar, tip, col, mod, pue, cab, man, pre, pes, gas;

    public BC() throws IOException {
        lista = new HashMap<Integer, Coche>();//Lista inicial para guardar datos

        //MODO OFFLINE
        //cargarBD(rutaCSV); //Carga en el programa la base de datos desde csv
        
        //MODO ONLINE
        cargarBD();
        
        aux = new HashMap<>();//Lista auxiliar para filtrar datos
        listaCoche = new ArrayList();//Lista final que se envia a la interfaz
    }

    private Connection conexion() {
        try {

            String url = "jdbc:sqlserver://lenovo-alvaro.database.windows."
                    + "net:1433;databaseName=SATD2018";
            String user = "huarteto";
            String pass = "Alva1234";

            Class.forName("com.microsoft.sqlserver.jdbc.SQLServerDriver");
            System.out.println("Driver cargado...");

            Connection conexion = DriverManager.getConnection(url, user, pass);

            if (conexion == null) {
                System.out.println("NULL");
            }
            System.out.println("Conexion a la base de datos establecida...");

            return conexion;
        } catch (SQLException sqle) {
            System.out.println("Sql Exception :" + sqle.getMessage());
        } catch (ClassNotFoundException e) {
            System.out.println("Class Not Found Exception :" + e.getMessage());
        }
        return null;
    }

    private void cargarBD(){
        boolean columna_nombres = true;
        Connection a = conexion(); //Conexion a SSMS1
        String concesionario = "";
        String marca = "";
        String tipo_coche = "";
        String color = "";
        String modelo = "";
        String puertas = "";
        String caballos = "";
        String manual = "";
        String precio = "";
        String peso = "";
        String gasolina = "";
        String total = "";
        int identificador = 0;
        try {
            con = consultar(a, RS, "concesionario");
            mar = consultar(a, RS, "Marca_Coche");
            tip = consultar(a, RS, "Tipo coche");
            col = consultar(a, RS, "color");
            mod = consultar(a, RS, "modelo");
            pue = consultar(a, RS, "puertas");
            cab = consultar(a, RS, "caballos");
            man = consultar(a, RS, "Manual(Si No(Automático))");
            pre = consultar(a, RS, "precio");
            pes = consultar(a, RS, "peso");
            gas = consultar(a, RS, "Gasolina/Diesel");

            while (con.next()) {
                concesionario = con.getString(1);
                total += concesionario + ";";
                if (mar.next()) {
                    marca = mar.getString(1);
                    total += marca + ";";
                }
                if (tip.next()) {
                    tipo_coche = tip.getString(1);
                    total += tipo_coche + ";";
                }
                if (col.next()) {
                    color = col.getString(1);
                    total += color + ";";
                }
                if (mod.next()) {
                    modelo = mod.getString(1);
                    total += modelo + ";";
                }
                if (pue.next()) {
                    puertas = pue.getString(1);
                    total += puertas + ";";
                }
                if (cab.next()) {
                    caballos = cab.getString(1);
                    if(Integer.parseInt(caballos)< CABALLOS_MIN) 
                        CABALLOS_MIN = Integer.parseInt(caballos) ;
                    if(Integer.parseInt(caballos)> CABALLOS_MAX) 
                        CABALLOS_MAX = Integer.parseInt(caballos) ;
                    total += caballos + ";";
                }
                if (man.next()) {
                    manual = man.getString(1);
                    total += manual + ";";
                }
                if (pre.next()) {
                    precio = pre.getString(1);
                    if(Integer.parseInt(precio)< PRECIO_MIN) 
                        PRECIO_MIN = Integer.parseInt(precio) ;
                    if(Integer.parseInt(precio)> PRECIO_MAX) 
                        PRECIO_MAX = Integer.parseInt(precio) ;
                    total += precio + ";";
                }
                if (pes.next()) {
                    peso = pes.getString(1);
                    if(Integer.parseInt(peso)< PESO_MIN) 
                        PESO_MIN = Integer.parseInt(peso) ;
                    if(Integer.parseInt(precio)> PESO_MAX) 
                        PESO_MAX = Integer.parseInt(peso) ;
                    total += peso + ";";
                }
                if (gas.next()) {
                    gasolina = gas.getString(1);
                    total += gasolina + "";
                }

                System.out.println(total);
                //Saltar la primera columna de nombres
                if (!columna_nombres) {
                    Coche coche = new Coche(total, identificador);
                    nuevo(lista, coche);
                    identificador++;
                } else {
                    columna_nombres = false;
                }
                total = "";
            }
        } catch (SQLException ex) {
            Logger.getLogger(BC.class.getName()).log(Level.SEVERE, null, ex);
        }
    }
    
    private ResultSet consultar(Connection con, ResultSet rs, String variable)
            throws SQLException {
        stm = con.createStatement();
        rs = stm.executeQuery("SELECT [" + variable + "] FROM  CochesBD");
        return rs;
    }
    
    private int comparaValor(Coche primero, Coche segundo){
        if(primero.devuelveValor() > segundo.devuelveValor())
            return -1;
        return 1;
    }

    /**
     * Metodo encargado de recorrer y cargar uno a uno los datos del CSV, se
     * salta la primera línea dando por hecho que están los nombres de las col.
     *
     * @param enlaceBD
     * @throws FileNotFoundException
     * @throws IOException
     */
    private void cargarBD(String enlaceBD) throws FileNotFoundException
            , IOException {
        FileReader lector = new FileReader(new File(enlaceBD));
        BufferedReader bufferContacto = new BufferedReader(lector);
        String linea;
        int identificador = 0;
        while ((linea = bufferContacto.readLine()) != null) {
            if (!linea.isEmpty()) {
                Coche coche = new Coche(linea, identificador);
                
                if(coche.devuelvePrecio()< PRECIO_MIN) 
                        PRECIO_MIN = coche.devuelvePrecio() ;
                if(coche.devuelvePrecio()> PRECIO_MAX) 
                        PRECIO_MAX = coche.devuelvePrecio() ;
                if(coche.devuelvePeso()< PESO_MIN) 
                        PESO_MIN = coche.devuelvePeso() ;
                if(coche.devuelvePeso()> PESO_MAX) 
                        PESO_MAX = coche.devuelvePeso() ;
                if(coche.devuelveCaballos()< CABALLOS_MIN) 
                        CABALLOS_MIN = coche.devuelveCaballos() ;
                    if(coche.devuelveCaballos()> CABALLOS_MAX) 
                        CABALLOS_MAX = coche.devuelveCaballos() ;
                nuevo(lista, coche);
            }
            identificador++;
        }
    }

    /**
     * Inserta en el HashMap el nuevo tipo de coche que recibe
     *
     * @param coche
     * @return
     */
    private void nuevo(Map m, Coche o) {
        m.put(o.devuelveId(), o);
    }

    private void nuevoLista(Coche coche, int index) {
        listaCoche.add(index, coche);
    }

    private void cargarLista(Map map) {
        int index = 0;
        for (int i = 0; i < map.size(); i++) {
            if (map.get(i) != null) {
                nuevoLista((Coche) map.get(i), index);
                index++;
            }
        }
    }

    /**
     * Filtra la lista aux y borra los que no son del tipo buscado s
     *
     * @param tipo
     */
    public void filtrarTipo_Coche(String tipo) {
        Iterator it = lista.entrySet().iterator();
        while (it.hasNext()) {
            Map.Entry e = (Map.Entry) it.next();
            Coche coche = (Coche) lista.get(e.getKey());
            lista.get(e.getKey());
            if (coche.devuelveTipo().equals(tipo)) {
                nuevo(aux, coche);
            }
        }
    }

    /**
     * Metodo encargado de buscar coches que tengan las mismas carcacteristicas
     * almacenandolos en una lista que despues devolverá, si se da el caso que
     * no encuentra ninguno por la variable combustible o automatico, se
     * calculara meduante porcentajes.
     *
     * @param marca
     * @param modelo
     * @param tipoCoche
     * @param automatico
     * @param combustible
     */
    public void bucarCoches(String marca, String modelo, String tipoCoche,
            boolean automatico, int combustible) {

    }

    public void iniciarListas() {
        aux.clear();
        listaCoche.clear();
    }

    public Map devuelveAux() {
        return aux;
    }

    public int devuelveTamano() {
        return aux.size();
    }

    public List devuelveTipo() {
        List list = new ArrayList();
        Iterator it = lista.entrySet().iterator();
        while (it.hasNext()) {
            boolean repetido = false;
            Map.Entry e = (Map.Entry) it.next();
            Coche coche = (Coche) lista.get(e.getKey());
            for (int i = 0; i < list.size(); i++) {
                if (comparaTipo(list, coche, i)) {
                    repetido = true;
                }
            }
            if (!repetido) {
                list.add(coche.devuelveTipo());
                repetido = false;
            }
        }
        return list;
    }
    
    public String calcularPrecio(String tipoCoche, String objetivo){
        //Filtra la lista auxiliar por el tipo de coche elegido
        filtrarTipo_Coche(tipoCoche);
         switch (objetivo) {

         //Minimizar coste viaje,Valor depende del peso, caballos y tipo motor
            case "Minimizar coste viaje":
                minimizarCosteViaje();
                break;
         // Minimizar Precio, Valor depende del precio
            case "Minimizar precio":
                minimizarPrecio();
                break;
            case "Maximizar comodidad":
                maximizarComodidad();
                break;
        }
         cargarLista(aux);//Pasa de Map a List una vez filtrado para poder ordenar
         Collections.sort(listaCoche, (Coche p1, Coche p2)
            -> new Integer(comparaValor(p1, p2)));

         
         return listar();
    }
    
    private void minimizarPrecio(){
        Iterator it = aux.entrySet().iterator();
        while (it.hasNext()) {
            Map.Entry e = (Map.Entry) it.next();
            Coche coche = (Coche) aux.get(e.getKey());
            coche.calcularMinPrecio(PRECIO_MIN, PRECIO_MAX);
            }
        }
    private void maximizarComodidad(){
        Iterator it = aux.entrySet().iterator();
        while (it.hasNext()) {
            Map.Entry e = (Map.Entry) it.next();
            Coche coche = (Coche) aux.get(e.getKey());
            coche.calcularMaxComodidad(PRECIO_MIN, PRECIO_MAX);
            }
        }
    private void minimizarCosteViaje(){
        Iterator it = aux.entrySet().iterator();
        while (it.hasNext()) {
            Map.Entry e = (Map.Entry) it.next();
            Coche coche = (Coche) aux.get(e.getKey());
            coche.calcularMinCosteViaje(PESO_MIN, PESO_MAX, CABALLOS_MIN
                    , CABALLOS_MAX);
            }
    }
        
    private String listar() {
        String cadena = "";
        for (int i = 0; i < listaCoche.size(); i++) {
            Coche name = (Coche) listaCoche.get(i);
            cadena += name.devuelveTipo()+" "+name.devuelveMarca()+" "
                     + name.devuelveModelo() + ". Puertas -> "+ name.devuelveNumPuertas() 
                    + ". Caballos ->" + name.devuelveCaballos()+". Peso ->"+name.devuelvePeso()
                    +". Automatico ->"+name.automatico()+". Gasolina ->"+name.gasolina()
                    + ". Precio final -> " + name.devuelvePrecio() + ".Puntuación "
                    +name.devuelveValor2Decimales() +"\n";

        }
        return cadena;
    }

    private boolean comparaTipo(List lista, Coche coche, int i) {
        return ((String) lista.get(i)).equals(coche.devuelveTipo());
    }

}
