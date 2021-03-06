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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import ru.rpunet.webmops.dao.PersonManager;
import ru.rpunet.webmops.model.Person;
import ru.rpunet.webmops.utils.PasswordService;
import ru.rpunet.webmops.utils.SystemUnavaliableException;
import ru.rpunet.webmops.utils.WebmopsActionBean;
import net.sourceforge.stripes.action.DefaultHandler;
import net.sourceforge.stripes.action.DontValidate;
import net.sourceforge.stripes.action.ForwardResolution;
import net.sourceforge.stripes.action.HandlesEvent;
import net.sourceforge.stripes.action.RedirectResolution;
import net.sourceforge.stripes.action.Resolution;
import net.sourceforge.stripes.action.UrlBinding;
import net.sourceforge.stripes.integration.spring.SpringBean;
import net.sourceforge.stripes.validation.LocalizableError;
import net.sourceforge.stripes.validation.Validate;
import net.sourceforge.stripes.validation.ValidationError;

/**
 * @author Andrew Diakin
 *
 */
@UrlBinding("/users/Login.action")
public class LoginActionBean extends WebmopsActionBean {
	
	private static Log log = LogFactory.getLog(LoginActionBean.class);
	
	@SpringBean
	private PasswordService passwordService;
	
	@Validate(required=true)
	private String login;
	
	@Validate(required=true)
	private String password;
	
	public String targetUrl;
	
	public String getLogin() {
		return this.login;
	}
	
	public void setLogin(String login) {
		this.login = login;
	}
	
	public String getPassword() {
		return this.password;
	}
	
	public void setPassword(String password) {
		this.password = password;
	}
	
	@DontValidate
	@DefaultHandler
	public Resolution index() {
		return new ForwardResolution("/Login.jsp");
	}
	
	@HandlesEvent("doLogin")
	public Resolution doLogin() {
		log.info("Validating user.");
	
		PersonManager pm = new PersonManager();
		Person user = pm.findPerson(login);
		String encryptedPassword = null;
		try {
			log.debug("Creating password hash");
			encryptedPassword = passwordService.encrypt(password);
		} catch (SystemUnavaliableException e) {
			e.printStackTrace();
		}
		
		if ( user == null ) {
			ValidationError error = new LocalizableError("userDoesNotExist");
			getContext().getValidationErrors().add("login", error);
			return getContext().getSourcePageResolution();
		} else if ( !user.getPassword().equals(encryptedPassword) ) {
			ValidationError error = new LocalizableError("incorrectPassword");
			getContext().getValidationErrors().add("password", error);
			return getContext().getSourcePageResolution();
		} else {
			log.info("User logger as " + user.getLogin());
			getContext().setUser(user);
			
			if ( this.targetUrl != null ) {
				return new RedirectResolution(this.targetUrl);
			} else {
				return new RedirectResolution(ru.rpunet.webmops.web.DefaultActionBean.class);
			}
		}
	}
	
}
