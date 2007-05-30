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

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.GenerationType;
import javax.persistence.Id;
import javax.persistence.JoinColumn;
import javax.persistence.ManyToOne;
import javax.persistence.Table;

import org.hibernate.validator.NotNull;

/**
 * File entiry
 * 
 * file has one package binded
 * 
 * @see Package 
 * @author Andrew Diakin
 *
 */

@Entity
@Table(name="files")
public class File implements Serializable {
	
	/**
	 * 
	 */
	private static final long serialVersionUID = 5028978754875967429L;

	@Id
	@GeneratedValue(strategy=GenerationType.IDENTITY)
	private Long id;
	
	@ManyToOne()
	@JoinColumn(name="package_id")
	private Package pkg;
	
	@NotNull
	@Column(name="name")
	private String name;
	
	public File() {
		
	}

	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}

	public Package getPackage() {
		return pkg;
	}

	public void setPackage(Package pkg) {
		this.pkg = pkg;
	}

	public Long getId() {
		return id;
	}
	
	
}
