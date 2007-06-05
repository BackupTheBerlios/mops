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

package ru.rpunet.webmops.utils;

import java.io.UnsupportedEncodingException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import sun.misc.BASE64Encoder;

/**
 * @author Andrew Diakin
 *
 */
public class PasswordServiceImpl implements PasswordService {

	private static Log log = LogFactory.getLog(PasswordService.class);
	
	private static PasswordServiceImpl instance;
	
	public PasswordServiceImpl() {
		log.info("Initilizing PasswordServiceImpl");
	}
	
	public synchronized String encrypt(String plaintext) throws SystemUnavaliableException {
		
		log.debug("Calling encrypt");
		
		MessageDigest md = null;
		
		try {
			md = MessageDigest.getInstance("SHA");
		} catch (NoSuchAlgorithmException e) {
			throw new SystemUnavaliableException(e.getMessage());
		}
		
		try {
			md.update(plaintext.getBytes("UTF-8"));
		} catch (UnsupportedEncodingException e) {
			throw new SystemUnavaliableException(e.getMessage());
		}
		
		byte raw[] = md.digest();
		String hash = (new BASE64Encoder()).encode(raw);
		
		return hash;
	
	}
	
	public static synchronized PasswordServiceImpl getInstance() {
		if (instance == null) {
			instance = new PasswordServiceImpl();
		}
		
		log.debug("Returning instance");
		
		return instance;
	}
	
}
