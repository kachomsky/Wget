import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;

public class HtmlToAsciiInputStream extends FilterInputStream{
	
	private boolean encontrado = false;
	byte[] b = new byte[100000]; //Array de bytes con un tamanyo para poder leer temporalmente el flujo de entrada y escribirlo en memoria
	
	
	public HtmlToAsciiInputStream(InputStream input){
		super(input);
	}
	
	
	@Override	
	public int read(){
		try {
			int r = super.read();
			
			if((char)r == '<'){
				this.encontrado = true;
			}
			
			if(!this.encontrado){
				return r;
			}
			
			if(this.encontrado){
				if((char)r == '>'){		
					this.encontrado = false;
				}
			}	
			
		} catch (IOException e) {			
			e.printStackTrace();
		}	
		return -2;
	}
	
	

	
}
