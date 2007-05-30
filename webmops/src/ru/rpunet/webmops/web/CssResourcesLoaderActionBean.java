/* Copyright 2007 Andrew Diakin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package ru.rpunet.webmops.web;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;


import net.sourceforge.stripes.action.DefaultHandler;
import net.sourceforge.stripes.action.Resolution;
import net.sourceforge.stripes.action.StreamingResolution;
import ru.rpunet.webmops.utils.WebmopsActionBean;

/**
 * Load css/js resources with context processing
 * 
 * @author Andrew Diakin
 *
 */
public class CssResourcesLoaderActionBean extends WebmopsActionBean {
	
	private static Log log = LogFactory.getLog(CssResourcesLoaderActionBean.class);
	
	private String style;
	
	// not used
	private String version = null;
	private StringBuffer data = new StringBuffer("");
	private String webPath;
	
	public String getVersion() {
		return this.version;
	}
	
	public void setVersion(String version) {
		this.version = version;
	}
	
	public String getStyle() {
		return this.style;
	}
	
	public void setStyle(String path) {
		this.style = path;
	}
	
	private void readResource() {
		log.debug("Reading css from " + style);
		BufferedReader in = null;
		try {
			in = new BufferedReader( new FileReader(style));
			String line;
			while ( (line = in.readLine()) != null ) {
				processLine(line);
			}
			
		} catch (FileNotFoundException e) {
			// there is nothing we can do but send clean data
			this.data.append("/** shit happens */");
		} catch (IOException e) {
			// same as previous...
			this.data.append("shit happens");
		} finally {
			try {
				if (in != null)
					in.close();
			} catch (Exception e) {
			}
		}
	}
	
	private void processLine(String line) {
		// search for every line contains url() instruction
		if ( line.contains("url(" ) ) {
			if ( line.contains("url(/") ) {
				// abs path
				data.append(line.replace("url(/", "url(" + webPath + "/") + "\n");
			} else {
				data.append(line.replace("url(", "url(" + webPath + "/") + "\n");
			}
		} else {
			// nothind to do
			data.append(line + "\n");
		}
	}
	
	@DefaultHandler
	public Resolution load() {
		if ( style.contains("/") ) {
			return new StreamingResolution("text/css", "");
		}
		this.webPath = getContext().getRequest().getContextPath();
		style = getContext().getServletContext().getRealPath("styles/" + style);
		log.debug("Processing css (" + webPath + ").");
		readResource();
		
		return new StreamingResolution("text/css", data.toString());
	}
}
