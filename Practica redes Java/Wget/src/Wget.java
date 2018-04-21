import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;

public class Wget {

	public static void main(String[] args){
		String urlString = ""; //La usamos para leer linea a linea la URL del fichero, almacenaremos aqui la url
		String file = "";
		int[] params = new int[4];//array donde ordenaremos los parametros
		params = new int[]{-1,-1,-1,-1};
		int contadorFicheros = 0;
		try{
			/****COMPROBAMOS LOS PARAMETROS QUE SE PASAN****/
			for(int i=0;i<args.length;i++){
				
				switch(args[i]){
					case "-f":
						if(args.length < 2 || args[i+1].charAt(0)=='-'){
							System.out.println("No se ha pasado el fichero");
						}else{
							file = args[i+1];
							params[0] = 0;
						}
						break;
					case "-a":
						params[1] = i;
						break;
					case "-z":
						params[2] = i;
						break;
					case "-gz":
						params[3] = i;
						break;						
				}
			}
			/******************************************************/
			BufferedReader reader = new BufferedReader(new FileReader(file)); //Lector con un fichero como entrada de parametro, leeremos los datos del fichero
			while((urlString = reader.readLine()) != null){	 //Mientras haya algo que leer en el fichero
				Fil f = new Fil(urlString, params, contadorFicheros);
				f.start();
				contadorFicheros++;
			}
			reader.close();
		}catch(FileNotFoundException urlnotfound){
			System.out.println("***** No s'ha establert conexio amb "+urlString+". Aquesta URL no esta disponible ******\n");
		}catch(Exception e){
			System.out.println("***** Ha ocurrido algun error *****");
		}
		
	}
}
