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

package ru.rpunet.webmops.importer;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLConnection;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import ru.rpunet.webmops.dao.FileManager;
import ru.rpunet.webmops.dao.PackageManager;
import ru.rpunet.webmops.model.Package;

/**
 * 
 * FIXME: REWRITE EVERYTHING!!!!
 * 
 * @author Andrew Diakin
 *
 */
public class SlackwarePackagesListReaderImpl implements PackageListReader {

	private static Log log = LogFactory.getLog(PackageListReader.class);
	
	private PackageManager packageDAO;
	private FileManager fileDAO;
	
	private Map<Long, Package> list;
	private String filePath;
	private boolean isPackageRecordEnd = false;
	private Package pkg;
	private String md5File;
	private Map<String, Long> map;
	private List<String> files;
	
	private boolean isMD5started = false;
	
	public SlackwarePackagesListReaderImpl() {
		this.list = new TreeMap<Long, Package>();
		this.map = new TreeMap<String, Long>();
		files = new ArrayList<String>();
		
		packageDAO = new PackageManager();
		fileDAO = new FileManager();
	}

	public void addFiles(List<String> fileList) {
		for (String f : fileList) {
			files.add(f);
		}
		
		downloadFiles(files, "/tmp/downloadpkg/");
	}
	

	public List<Package> getPackages() {
		parseFile();
		return Collections.unmodifiableList(new ArrayList<Package>(list.values()));
	}
	
	private void parseFile() {
		try {
			BufferedReader in = new BufferedReader(new FileReader(this.filePath));
			String line;
			while ((line = in.readLine()) != null) {
				if (pkg == null) {
					pkg = new Package();
				}
				processLine(line);
			
				if (isPackageRecordEnd) {
					/**
					 * all fields found, save package
					 */
					if (pkg.getName() != null) {
						list.put(pkg.getId(), pkg);
						
						// saving package to database
						packageDAO.makePersistent(pkg);
					}
					pkg = null;
				}
			}
			in.close();

			/*
			 * load md5 for packages
			 */
			BufferedReader md5in = new BufferedReader(new FileReader(this.md5File));
			line = null;
			while ((line = md5in.readLine()) != null) {
				processMD5Line(line.trim());
			}
			md5in.close();		
			
			readFileList("/tmp/MANIFEST");
			
		} catch (IOException e) {
			System.out.println("Error reading file.");
		} catch (MalformedTokenException e) {
			e.printStackTrace();
		}
	}
	

	
	private void processMD5Line(String line) {
		if (line.contains("MD5 message digest ")) {
			isMD5started = true;
			return;
		}
		
		if (isMD5started || !isMD5started) {
			String parts[] = line.split(" \\.");
			//String parts[] = line.split(" ");
			String currentMD5;
			String currentFile;
			
			try {
				currentMD5 = parts[0];
				currentFile = "." + parts[1];
				if ( currentFile == null && currentMD5 == null )
					return;
				
				if ( currentFile.equalsIgnoreCase("/PACKAGES.TXT") ||
					 currentFile.equalsIgnoreCase("/FILELIST.TXT") ||
					 currentFile.equalsIgnoreCase("/MANIFEST.bz2"))
					return;

				 Package p = list.get( map.get(currentFile));
				 if (p != null) {
					 p.setMd5(currentMD5);
				 }
					
			} catch (IndexOutOfBoundsException e) {
			
			}
		}
	}
	
	private void processLine(String line) throws MalformedTokenException {
		TokenType currentToken;
		String currentKey;
		String currentValue;
		String[] tokens = null; 
		
		if (line.trim().length() == 0) {
			currentToken = TokenType.DELIMITER_TOKEN;
			isPackageRecordEnd = true;
			return;
		} else {
			tokens = line.trim().split(":");
		}
		try {
			currentKey = tokens[0];
			currentValue = tokens[1];
		
		
			if (currentKey == null || currentValue == null)
				return;
			
			currentToken = getToken(currentKey);
			switch (currentToken) {
			case NAME_TOKEN:
				isPackageRecordEnd = false;
				parseName(currentValue.trim());
				pkg.setName(currentValue.trim());
				pkg.setFileName(currentValue.trim());
				break;
				
			case LOCATION_TOKEN:
				pkg.setLocation(currentValue.trim());
				break;
				
			case SIZE_COMP_TOKEN:
				//pkg.setCompressedSize(currentValue.trim());
				break;
				
			case SIZE_INST_TOKEN:
				//pkg.setInstalledSize(currentValue.trim());
				break;
				
			case DELIMITER_TOKEN:
				isPackageRecordEnd = true;
				break;
				
			case DESCRIPTION_START_TOKEN:
				break;
				
			case DESCRIPTION_TOKEN:
				if (pkg.getDescription() == null) {
					pkg.setDescription(currentValue);
				} else {
					pkg.setDescription(pkg.getDescription().concat(currentValue));
				}
				
				break;
				
			case UNKNOWN_TOKEN:
			default:
				break;
			}
		
		}
		catch (ArrayIndexOutOfBoundsException e) {
		}
		
	}
	
	
	/**
	 * setup package info (name, version, build, arch)
	 * @param name
	 */
	private void parseName(String name) {
		String[] parts;
		
		name = name.replace(".tgz", "");
		parts = name.split("-");
	
		List<String> tags = new ArrayList<String>(Arrays.asList(parts));
		
		if (tags != null) {
			if (tags.size() != 0) {
				int i = ((tags.size()) - 1);			
				pkg.setBuild(tags.get(i));
				i--;
				pkg.setArch(tags.get(i));
				i--;
				pkg.setVersion(tags.get(i));
				i--;
				// everithing after version is package name
				int k = 0;
				String pkgname = "";
				while (k <= i) {
					pkgname += tags.get(k);
					k++;
					if (k <= i)
						pkgname += "-";
					//i--;
				}
				pkg.setName(pkgname);
			}
			
		}
		
	
	}
	
	private TokenType getToken(String line) {
		line = line.trim();
		
		if (line.startsWith("PACKAGE NAME"))
			return TokenType.NAME_TOKEN;
		
		if (line.startsWith("PACKAGE LOCATION"))
			return TokenType.LOCATION_TOKEN;
		
		if (line.startsWith("PACKAGE SIZE (compressed)"))
			return TokenType.SIZE_COMP_TOKEN;
		
		if (line.startsWith("PACKAGE SIZE (uncompressed)"))
			return TokenType.SIZE_INST_TOKEN;
		
		if (line.startsWith("PACKAGE DESCRIPTION"))
			return TokenType.DESCRIPTION_START_TOKEN;
		
		/*
		 * everything starts from [a-zA-Z]: is package description
		 */
		if (line.matches("(.*?)+"))
			return TokenType.DESCRIPTION_TOKEN;
		
		if (line.trim().equalsIgnoreCase(" ") && line.trim().length() == 0)
			return TokenType.DELIMITER_TOKEN;
		
		return TokenType.UNKNOWN_TOKEN;
	}


	public Map<String, Long> getPackagesIndexByPath() {
		return this.map;
	}
	
	private void readFileList(String file) {
		boolean isPackageListStart = false;
		boolean isInHeader = false;
		boolean isInFileList = false;
		
		String startHeader = "++==";
		String headerDelim = "||   Package:";
		
		try {
			BufferedReader in = new BufferedReader(new FileReader(file));
			String line;
			String currentPackage = null;
			while ((line = in.readLine()) != null) {
				//processFileListLine(line.trim());
				line = line.trim();
				if (line.startsWith(startHeader)) {
					if ( !isInHeader ) {
						isPackageListStart = true;
						isInHeader = true;
					} else {
						isInHeader = false;
					}
				}
				
				if (line.trim().equalsIgnoreCase("")) {
					isPackageListStart = false;
				}
				
				if ( isPackageListStart ) {
					if ( line.startsWith("-") || line.startsWith("d") ||
							line.startsWith("b") || line.startsWith("h") ||
							line.startsWith("c") || line.startsWith("l") ||
							line.startsWith("p")
						) {
						isInFileList = true;
					} else {
						isInFileList = false;
					}
				}
				
				if ( isInHeader ) {
					if ( line.indexOf(headerDelim) != -1 ) {
						currentPackage = line.replace(headerDelim, "").trim();
						//System.err.println("Package " + line.replace(headerDelim, "").trim());
					} 
				}
				
				if ( !isInHeader && (isPackageListStart && isInFileList) ) {
					// line with filename
					String parts[] = line.split("\\s+");
					String _fileName = null;
					try {
						_fileName = parts[5].trim();
					} catch (IndexOutOfBoundsException e) {
					}
					
					if (_fileName != null && !_fileName.equalsIgnoreCase("./")
							&& !_fileName.startsWith("install/")) {
						Package pkg = list.get(map.get(currentPackage));
						if (pkg != null ) {
							//pkg.addFileToList(_fileName);
						}	
					}
					//System.out.println(_fileName.trim());
				}
				
			}
			in.close();
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}


	/**
	 * Download files with packages info
	 * @param fileList List<> of files
	 * @param outputDir
	 */
	private void downloadFiles(List<String> fileList, String outputDir) {
		String outputDirName = outputDir.endsWith("/") ? outputDir : outputDir + "/";
		
		File f = new File(outputDirName);
		if (!f.exists()) {
			// create output dir
			if (!f.mkdirs()) {
				System.err.println("Failed to create " + outputDirName);
			}
		}
		
		List<String> ftpFiles = new ArrayList<String>();
		List<String> httpFiles = new ArrayList<String>();
		
		for (String file : fileList) {
			if (file.startsWith("ftp://")) {
				ftpFiles.add(file);
			} else if (file.startsWith("http://")) {
				httpFiles.add(file);
			}
		}
		
		if (httpFiles.size() > 0) {
			log.debug("Downloading files from http.");
			downloadHTTP(httpFiles, outputDirName);
		}
		
		if (ftpFiles.size() > 0) {
			log.debug("Downloading files from ftp.");
			downloadFTP(ftpFiles, outputDirName);
		}
		
		
	}
	
	private void downloadHTTP(List<String> files, String outputDirName) {
		
		for (String file : files) {
			OutputStream out = null;
			URLConnection conn = null;
			InputStream in = null;
			URL url = null;
			
			try {
				String outputFileName;
								
				url = new URL(file);
				outputFileName = getFileFromURL(url.getPath());
				
				out = new BufferedOutputStream(
						new FileOutputStream(outputDirName + outputFileName));
				
				conn = url.openConnection();
				in = conn.getInputStream();
				byte[] buffer = new byte[4 * 1024]; // 4K buffer
				int numRead;
				long numWritten = 0;
				
				log.debug("Downloading file to " + outputDirName + outputFileName);
				
				while ((numRead = in.read(buffer)) != -1) {
					out.write(buffer, 0, numRead);
					numWritten += numRead;
				}
			
				
			} catch (MalformedURLException e) {
				System.err.println("Invalid url " + file);				
			} catch (IOException e) {
				System.err.println("Failed to create rw streams.");
			} finally {
				try {
					if (in != null)
						in.close();
					if (out != null)
						out.close();
				} catch (IOException ioe) {
					
				}
				
			}
		}
		
		
	} 
	
	private void downloadFTP(List<String> files, String outputDir) {
		URL url = null;
		
		for (String file : files) {
			BufferedInputStream in = null;
			OutputStream out = null;
	
			try {
				
				url = new URL(file);
				
				String outputFileName = getFileFromURL(url.getPath());
				log.debug("Downloading file to " + outputDir + outputFileName);
				URLConnection conn = url.openConnection();
				in = new BufferedInputStream(
						conn.getInputStream());
				
				out = new BufferedOutputStream(
						new FileOutputStream(outputDir + outputFileName));
				
				
				byte[] buffer = new byte[4 * 1024]; // 4K buffer
				int numRead;
				long numWritten = 0;
				
				while ((numRead = in.read(buffer)) != -1) {
					out.write(buffer, 0, numRead);
					numWritten += numRead;
				}
				
				
			} catch (MalformedURLException e) {
				log.debug("Server refused connection.");
			} catch (IOException e) {
			} finally {
				try {
					if (in != null)
						in.close();

					if (out != null)
						out.close();
					
				} catch (IOException e) {
				}
		
			}
			
		}
	
		throw new NoSuchMethodError("No FTP downloads yet.");
	}
	
	private String getFileFromURL(String path) {
		String outputFile = "";

		int lastSlashIndex = path.lastIndexOf('/');
		if (lastSlashIndex >= 0 &&
				lastSlashIndex < path.length() - 1) {
			outputFile = path.substring(lastSlashIndex + 1);
		}
		
		log.debug("Returning file " + outputFile);
		return (outputFile.length() > 0) ? outputFile : null;
	}
	

}
