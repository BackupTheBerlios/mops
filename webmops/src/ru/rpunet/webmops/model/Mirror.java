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
   
package ru.rpunet.webmops.model;

import java.io.Serializable;

import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.GenerationType;
import javax.persistence.JoinColumn;
import javax.persistence.ManyToOne;
import javax.persistence.Table;
import javax.persistence.Id;

import org.hibernate.validator.NotNull;

/**
 * Mirror entity
 * mirrors can have only one distribution
 * 
 * @author andrew
 * @see Distribution
 */
@Entity
@Table(name="mirrors")
public class Mirror implements Serializable {

	/**
	 * 
	 */
	private static final long serialVersionUID = 5962706001834876189L;

	@Id
	@GeneratedValue(strategy=GenerationType.IDENTITY)
	private Long Id;
	
	@NotNull
	private String name;
	
	@NotNull
	private String url;
		/**
	 * mirror type, only HTTP and FTP supported
	 */
	@NotNull
	private String type;
	
	@NotNull
	@ManyToOne
	@JoinColumn(name="distribution_id")
	private Distribution distrubution;

	public Distribution getDistrubution() {
		return distrubution;
	}

	public void setDistrubution(Distribution distrubution) {
		this.distrubution = distrubution;
	}

	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}


	public String getType() {
		return type;
	}

	public void setType(String type) {
		this.type = type;
	}

	public String getUrl() {
		return url;
	}

	public void setUrl(String url) {
		this.url = url;
	}

	public Long getId() {
		return Id;
	}
	
	
}
