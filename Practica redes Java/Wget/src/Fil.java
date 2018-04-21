import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;
import java.util.zip.GZIPOutputStream;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

public class Fil extends Thread{
	private String urlString;
	private int[] params;
	private int contadorFicheros;
	
	public Fil(String url, int[] params, int contadorFicheros){
		this.urlString = url;
		this.params = params;
		this.contadorFicheros = contadorFicheros;
	}

	public void run(){
		URL url; //Objeto url que tendra donde pondremos las URL del fichero a leer 
		OutputStream os = null; //Objeto tipo FileOutputStream para poder escribir en memoria los bytes que leamos de InputStream
		InputStream input;
		int r;
		String urlStr;
		String zipFileStr = "";
		
		try{
			url = new URL(this.urlString); //instanciamos un objeto URL
			System.out.println(url);
			
			/**** TRATAMOS EL SUFIJO DE LOS ARCHIVOS Y SU NOMBRE ***********/
			urlStr = new File(url.getPath().toString()).getName();
			if(urlStr == ""){ //Si nos dan una URL donde no haya fichero a descargar, usaremos un fichero llamado index para descargar el contenido
				urlStr = "index"+this.contadorFicheros+".html";
			}else{
				urlStr = urlStr.split("[.]")[0]+this.contadorFicheros+"."+urlStr.split("[.]")[1];
			}
			/***************************************************************/
			
			input = url.openConnection().getInputStream();
			/*** APLICAMOS EL FILTRO -a ***/
			if(params[1]!= -1){
				System.out.println(url);
				if(url.openConnection().getContentType().contains("text/")){
					urlStr = urlStr+".asc";
					input = new HtmlToAsciiInputStream(url.openConnection().getInputStream());//Conectamos a la URL y obtenemos un flujo de lectura gracias a InputStream			
					System.out.println("**** Se aplicara el filtro a este fichero "+urlStr+" ****");					
				}
			}
			
			/***** COMPROBAMOS QUE PARAMETROS SE HAN PASADO Y CONSTRUIMOS EL NOMBRE DEL FICHERO****/
			
			if(params[2]!=-1){
				zipFileStr = urlStr;
				urlStr = urlStr+".zip";
				
			}
			
			if(params[3]!=-1){
				zipFileStr = urlStr;
				urlStr = urlStr+".gz";
			}
			
			/*****************************************************************************/	
			os = new FileOutputStream(urlStr);
			
			
			/**** APLICAMOS EL FILTRO -z ****/
			if(params[2] != -1){				
				System.out.println("Comprimir en Zip");				
				ZipEntry zip = new ZipEntry(zipFileStr);				
				os = new ZipOutputStream(os);
				((ZipOutputStream) os).putNextEntry(zip);	
			}
			
			/**** APLICAMOS EL FILTRO -gz*****/
			if(params[3] != -1){
				System.out.println("Comprimir en Gzip");
				os = new GZIPOutputStream(os);
			}

			
			while((r = input.read()) != -1){
				if(r != -2){
					os.write(r);
				}
			}
			
			input.close();
			os.close();
		}catch(FileNotFoundException urlnotfound){
			System.out.println("***** No s'ha establert conexio amb "+urlString+". Aquesta URL no esta disponible ******\n");
		}catch(Exception e){
			e.printStackTrace();
			System.out.println("***** Ha ocurrido algun error *****");
		}
	}
}
