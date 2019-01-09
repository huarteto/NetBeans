package Proyecto;

/**
 *
 * @author Diego Alvarez
 * 
 * @version 0.1
 * @since 11/07/17
 * Esta clase es temporal, modificar para cargar desde fichero
 */
public class FieldsLoad {
    private static double[] TARGET_1835 = {0.5, 0.1, 0.3, 0.1};
    private static double[] TARGET_3650 = {0.5, 0.2, 0.2, 0.1};
    private static double[] TARGET_51100 = {0.3, 0.1, 0.3, 0.3};
    
    public static double[] getLoad(int _index) {
        double[] respuesta = null;
        switch(_index){
            case 0:
                respuesta = TARGET_1835;
                break;
            case 1:
                respuesta = TARGET_3650;
                break;
            case 2:
                respuesta = TARGET_51100;
                break;
            default:
                respuesta = TARGET_1835;
        }

        return respuesta;
    }
    
    /**
     * Devuelve el elemento Target pro defecto.
     * 
     * @return 
     */
    public static double getDefault(){
        return TARGET_1835[3];
    }
    
}
